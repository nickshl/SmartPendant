//******************************************************************************
//  @file ProgrammSender.cpp
//  @author Nicolai Shlapunov
//
//  @details ProgrammSender: User ProgrammSender Class, implementation
//
//  @copyright Copyright (c) 2016, Devtronic & Nicolai Shlapunov
//             All rights reserved.
//
//  @section SUPPORT
//
//   Devtronic invests time and resources providing this open source code,
//   please support Devtronic and open-source hardware/software by
//   donations and/or purchasing products from Devtronic.
//
//******************************************************************************

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "ProgrammSender.h"

#include "Application.h"

#include "fatfs.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
ProgrammSender& ProgrammSender::GetInstance()
{
  static ProgrammSender pgmsenderscr;
  return pgmsenderscr;
}

// *****************************************************************************
// ***   ProgrammSender Setup   ************************************************
// *****************************************************************************
Result ProgrammSender::Setup(int32_t y, int32_t height)
{
  // Fill menu_items
  for(uint32_t i = 0u; i < NumberOf(menu_items); i++)
  {
    menu_items[i].text = str[i];
    menu_items[i].n = sizeof(str[i]);
  }
  // Set callback
  menu.SetCallback(AppTask::GetCurrent(), this, reinterpret_cast<CallbackPtr>(ProcessMenuOkCallback), reinterpret_cast<CallbackPtr>(ProcessMenuCancelCallback));
  // Setup menu
  menu.Setup(menu_items, NumberOf(menu_items), 0, y, display_drv.GetScreenW(), height - Font_8x12::GetInstance().GetCharH() * 2u - BORDER_W*2);
  // Setup text box
  text_box.Setup(0, y, display_drv.GetScreenW(), height - Font_8x12::GetInstance().GetCharH() * 2u - BORDER_W*2);

  // Fill all windows
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    // Real position
    dw_real[i].SetParams(BORDER_W + ((display_drv.GetScreenW() - BORDER_W * 4) / 3 + BORDER_W) * i, y + height - Font_8x12::GetInstance().GetCharH() * 2u - BORDER_W, (display_drv.GetScreenW() - BORDER_W * 4) / 3, Font_8x12::GetInstance().GetCharH() * 2u, 7u, grbl_comm.GetUnitsPrecision());
    dw_real[i].SetBorder(BORDER_W / 2, COLOR_GREY);
    dw_real[i].SetDataFont(Font_8x12::GetInstance());
    dw_real[i].SetNumber(0);
    dw_real[i].SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT, Font_6x8::GetInstance());
    // Axis Name
    dw_real_name[i].SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_6x8::GetInstance());
    dw_real_name[i].Move(dw_real[i].GetStartX() + BORDER_W, dw_real[i].GetStartY() + BORDER_W);
  }

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result ProgrammSender::Show()
{
  // Set encoder callback handler(before menu show since menu will handle it also)
  InputDrv::GetInstance().AddEncoderCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessEncoderCallback), this, enc_cble);

  // Run button
  left_btn.SetParams("Run", left_btn.GetStartX(), left_btn.GetStartY(), (display_drv.GetScreenW()- BORDER_W * 2) / 3, left_btn.GetHeight(), true);
  // Stop button
  right_btn.SetParams("Stop", right_btn.GetEndX() - left_btn.GetWidth() + 1, right_btn.GetStartY(), left_btn.GetWidth(), right_btn.GetHeight(), true);
  // Open button
  open_btn.SetParams("Open", left_btn.GetEndX() + BORDER_W + 1, left_btn.GetStartY(), left_btn.GetWidth(), left_btn.GetHeight(), true);
  open_btn.SetCallback(AppTask::GetCurrent());

  // Show text box
  text_box.Show(100);

  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].Show(100);
    dw_real_name[i].Show(100);
  }

  // Open button
  open_btn.Show(102);
  // Run button
  left_btn.Show(102);
  // Stop button
  right_btn.Show(102);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result ProgrammSender::Hide()
{
  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);

  // Hide menu
  menu.Hide();
  // Hide text box
  text_box.Hide();
  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].Hide();
    dw_real_name[i].Hide();
  }
  // Go button
  left_btn.Hide();
  // Reset button
  right_btn.Hide();
  // Open button
  open_btn.Hide();

  // Reinit Soft Buttons to change their size back
  Application::GetInstance().InitSoftButtons();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result ProgrammSender::TimerExpired(uint32_t interval)
{
  // Update left & right button text
  Application::GetInstance().UpdateLeftButtonText();
  Application::GetInstance().UpdateRightButtonText();

  // Update numbers with current position and position difference
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].SetNumber(grbl_comm.GetAxisPosition(i));
  }

  if(run)
  {
    if(grbl_comm.GetState() == GrblComm::HOLD)
    {
      Application::GetInstance().EnableScreenChange();
    }
    else
    {
      Application::GetInstance().DisableScreenChange();
    }

    // If ID is zero - we didn't send any commands yet
    GrblComm::status_t result = (id != 0u) ? grbl_comm.GetCmdResult(id) : GrblComm::Status_OK;
    // If result of previous command is ok
    if((result == GrblComm::Status_OK) || (result == GrblComm::Status_Next_Cmd_Executed))
    {
      // Buffer for command
      char cmd[128u];
      // Since all program commands have striped out CR LF, we have to add it
      snprintf(cmd, NumberOf(cmd), "%s\r\n", text_box.GetSelectedStringText());
      // Send new command
      if(grbl_comm.SendCmd(cmd, id) == Result::RESULT_OK)
      {
        int32_t select = text_box.GetSelect();
        // Go to next line
        text_box.Select(select + 1);
        // Can't go further - end of program
        if(select == text_box.GetSelect())
        {
          // Clear run flag
          run = false;
        }
      }
    }
    else if(result == GrblComm::Status_Cmd_Not_Executed_Yet)
    {
      ; // Wait until command will be executed
    }
    else // In case of any error - stop executing program
    {
      // Clear run flag
      run = false;
    }
  }
  else
  {
    // Enable buttons back
    open_btn.Enable();
    // Enable screen change if program isn't running
    Application::GetInstance().EnableScreenChange();
    // Process it
    if(enc_val != 0)
    {
      // Set text to text box
      //text_box.Scroll(text_box.GetScroll() + enc_val);
      text_box.Select(text_box.GetSelect() + enc_val);
      // Clear encoder value
      enc_val = 0;
    }
  }
  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Private: ProcessMenuOkCallback function   *****************************
// *****************************************************************************
Result ProgrammSender::ProcessMenuOkCallback(ProgrammSender* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ProgrammSender& ths = *obj_ptr;

    // Hide the menu
    ths.menu.Hide();

    // If buffer was previously allocated
    if(ths.text != nullptr)
    {
      // Hide text box before delete buffer
      ths.text_box.Hide();
      // Delete previously allocated buffer
      delete [] ths.text;
    }

    // Open file
    FRESULT fres = f_open(&SDFile, ths.menu_items[(uint32_t)ptr].text, FA_OPEN_EXISTING | FA_READ);
    // Write data to file
    if(fres == FR_OK)
    {
      // Get file size
      uint32_t fsize = f_size(&SDFile) + 1u;
      // Allocate memory for data
      ths.text = new char[fsize];
      // Check if allocation was successful
      if(ths.text != nullptr)
      {
        // Read bytes
        UINT wbytes = 0u;
        // Read text
        fres = f_read(&SDFile, ths.text, fsize, &wbytes);
        // And null-terminator to it
        ths.text[wbytes] = 0x00;
        // Set text to text box
        ths.text_box.SetText(ths.text);
      }
      else
      {
        // If memory allocation operation isn't successful set text
        ths.text_box.SetText("; Program is too big!");
      }
    }
    else
    {
      // If memory allocation operation isn't successful set text
      ths.text_box.SetText("; Error open file!");
    }
    // Close file
    if(fres == FR_OK) fres = f_close(&SDFile);

    // And show it
    ths.text_box.Show(100);
    // Left button
    ths.left_btn.Show(102);
    // Right button
    ths.right_btn.Show(102);
    // Open button
    ths.open_btn.Show(102);

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ProcessMenuCancelCallback function   *************************
// *****************************************************************************
Result ProgrammSender::ProcessMenuCancelCallback(ProgrammSender* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ProgrammSender& ths = *obj_ptr;

    // Hide the menu
    ths.menu.Hide();
    // And show textbox
    ths.text_box.Show(100);
    // Run button
    ths.left_btn.Show(102);
    // Stop button
    ths.right_btn.Show(102);
    // Open button
    ths.open_btn.Show(102);

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result ProgrammSender::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  // Process Run button. Since we can call this handler after press of physical
  // button, we have to check if Run button is active.
  if(ptr == &left_btn)
  {
    // If we already run program
    if(run)
    {
      result = Result::ERR_UNHANDLED_REQUEST; // For Application to handle it
    }
    else
    {
      // Clear id to run program
      id = 0u;
      // Set run flag to start program streaming
      run = true;
      // Disable buttons while program is running
      open_btn.Disable();
      // Disable screen change if program is running
      Application::GetInstance().DisableScreenChange();
    }
  }
  // Process Reset button
  else if(ptr == &right_btn)
  {
    // Clear run flag
    run = false;
    // For Application to handle it(Stop/Reset)
    result = Result::ERR_UNHANDLED_REQUEST;
  }
  // Process Reset button
  else if((ptr == &open_btn) && (open_btn.IsActive()))
  {
    // Stop timer to prevent queue overflow since SD card operations can take some
    // time.
    AppTask::GetCurrent()->StopTimer();

    // Reinit SD card
    BSP_SD_Init();

    // Mount SD
    FRESULT res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
    DIR dir;

    uint32_t time_ms = 0;

    // Open the directory
    if(res == FR_OK)
    {
      time_ms = RtosTick::GetTimeMs();
      res = f_opendir(&dir, "/");
      time_ms = RtosTick::GetTimeMs() - time_ms;
    }

    uint32_t idx = 0u;

    if(res == FR_OK)
    {
      FILINFO fno;
      for(;;)
      {
        res = f_readdir(&dir, &fno);                  // Read a directory item
        if (res != FR_OK || fno.fname[0] == 0) break; // Break on error or end of dir
        if(!(fno.fattrib & AM_DIR))                   // It is a directory
        {
          menu_items[idx].str.SetString(menu_items[idx].text, menu_items[idx].n, "%s", fno.fname);
          idx++;
          if(idx == NumberOf(menu_items)) break;
        }
      }
      f_closedir(&dir);
    }
    // Fill menu_items
    for(uint32_t i = idx; i < NumberOf(menu_items); i++)
    {
      str[i][0] = '\0';
    }

    // Restart timer
    AppTask::GetCurrent()->StartTimer();

    // Hide text box before open menu
    text_box.Hide();
    // Go button
    left_btn.Hide();
    // Reset button
    right_btn.Hide();
    // Open button
    open_btn.Hide();

    // Set menu items count
    menu.SetCount(idx);
    // Show menu
    menu.Show(100);

    // Clear current position
    idx = 0u;
  }
  else
  {
    ; // Do nothing - MISRA rule
  }

  // Return result
  return result;
}

// *************************************************************************
// ***   Private: ProcessEncoderCallback function   ************************
// *************************************************************************
Result ProgrammSender::ProcessEncoderCallback(ProgrammSender* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ProgrammSender& ths = *obj_ptr;
    // Cast pointer itself to integer value and change current encoder value
    ths.enc_val += (int32_t)ptr;
    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *************************************************************************
// ***   Private constructor   *********************************************
// *************************************************************************
ProgrammSender::ProgrammSender() : left_btn(Application::GetInstance().GetLeftButton()),
                                   right_btn(Application::GetInstance().GetRightButton()) {};
