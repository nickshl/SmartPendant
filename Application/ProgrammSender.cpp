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
// ***   ProgrammSender Setup   ***********************************************
// *****************************************************************************
Result ProgrammSender::Setup(int32_t y, int32_t height)
{
  // Run button
  left_btn.SetParams("Run", 0, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3, (display_drv.GetScreenW()- BORDER_W * 2) / 3, Font_8x12::GetInstance().GetCharH() * 3, true);
  left_btn.SetCallback(AppTask::GetCurrent());
  // Stop button
  open_btn.SetParams("Open", left_btn.GetEndX() + BORDER_W + 1, left_btn.GetStartY(), left_btn.GetWidth(), left_btn.GetHeight(), true);
  open_btn.SetCallback(AppTask::GetCurrent());
  // Stop button
  right_btn.SetParams("Stop", display_drv.GetScreenW() - left_btn.GetWidth(), left_btn.GetStartY(), left_btn.GetWidth(), left_btn.GetHeight(), true);
  right_btn.SetCallback(AppTask::GetCurrent());

  // Fill menu_items
  for(uint32_t i = 0u; i < NumberOf(menu_items); i++)
  {
    menu_items[i].text = str[i];
    menu_items[i].n = sizeof(str[i]);
  }
  // Set callback
  menu.SetCallback(AppTask::GetCurrent(), this, reinterpret_cast<CallbackPtr>(ProcessMenuOkCallback), reinterpret_cast<CallbackPtr>(ProcessMenuCancelCallback));
  // Setup menu
  menu.Setup(menu_items, NumberOf(menu_items), 0, y, display_drv.GetScreenW(), height - left_btn.GetHeight() - BORDER_W);
  // Setup text box
  text_box.Setup(0, y, display_drv.GetScreenW(), height - left_btn.GetHeight() - BORDER_W);

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
  // Set callback handler for left and right buttons
  InputDrv::GetInstance().AddButtonsCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this, InputDrv::BTNM_LEFT | InputDrv::BTNM_RIGHT, btn_cble);

  // Show text box
  text_box.Show(100);
  // Run button
  left_btn.Show(102);
  // Stop button
  right_btn.Show(102);
  // Open button
  open_btn.Show(102);

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
  // Delete buttons callback handler
  InputDrv::GetInstance().DeleteButtonsCallbackHandler(btn_cble);

  // Hide menu
  menu.Hide();
  // Hide text box
  text_box.Hide();
  // Go button
  left_btn.Hide();
  // Reset button
  right_btn.Hide();
  // Open button
  open_btn.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result ProgrammSender::TimerExpired(uint32_t interval)
{
  // Update left button text
  if(grbl_comm.GetState() == GrblComm::RUN)
  {
    left_btn.SetString("Hold");
  }
  else
  {
    left_btn.SetString("Run");
  }

  if(run)
  {
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
          // Enable buttons back
          open_btn.Enable();
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
      // Enable buttons back
      open_btn.Enable();
      // Clear run flag
      run = false;
    }
  }
  else
  {
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

    // Open file
    FRESULT fres = f_open(&SDFile, ths.menu_items[(uint32_t)ptr].text, FA_OPEN_EXISTING | FA_READ);
    // Write data to file
    if(fres == FR_OK)
    {
      // Hide the menu
      ths.menu.Hide();

      // Read bytes
      UINT wbytes = 0u;
      // Read text
      fres = f_read(&SDFile, ths.text, sizeof(ths.text), &wbytes);
      // Check if we read all program and clear it
      if(wbytes >= sizeof(ths.text))
      {
        strncpy(ths.text, "; Program is too big!", sizeof(ths.text));
        wbytes = strlen(ths.text);
      }
      // And null-terminate it
      ths.text[wbytes] = 0x00;
      // Set text to text box
      ths.text_box.SetText(ths.text);
      // And show it
      ths.text_box.Show(100);
      // Run button
      ths.left_btn.Show(102);
      // Stop button
      ths.right_btn.Show(102);
      // Open button
      ths.open_btn.Show(102);
    }
    // Close file
    if(fres == FR_OK) fres = f_close(&SDFile);

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
  // Process Run button. Since we can call this handler after press of physical
  // button, we have to check if Run button is active.
  if(ptr == &left_btn)
  {
    // Disable buttons while program is running
    open_btn.Disable();
    // If we already run program
    if(run && grbl_comm.GetState() == GrblComm::RUN)
    {
      grbl_comm.Hold(); // Send Hold command for pause
    }
    else if(run)
    {
      grbl_comm.Run(); // Send Run command for continue
    }
    else
    {
      // Set run flag to start program streaming
      run = true;
    }
  }
  // Process Reset button
  else if(ptr == &right_btn)
  {
    // Enable buttons back
    open_btn.Enable();
    // Send Stop command
    grbl_comm.Stop();
    // clear run flag
    run = false;
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
  }

  // Always good
  return Result::RESULT_OK;
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

// *****************************************************************************
// ***   Private: ProcessButtonCallback function   *****************************
// *****************************************************************************
Result ProgrammSender::ProcessButtonCallback(ProgrammSender* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ProgrammSender& ths = *obj_ptr;
    // Get pressed button
    InputDrv::ButtonCallbackData btn = *((InputDrv::ButtonCallbackData*)ptr);

    // UI Button pointer
    UiButton *ui_btn = nullptr;

    // Find button pointer
    if(btn.btn == InputDrv::BTN_LEFT)       ui_btn = &ths.left_btn;
    else if(btn.btn == InputDrv::BTN_RIGHT) ui_btn = &ths.right_btn;
    else; // Do nothing - MISRA rule

    // If button object found
    if(ui_btn != nullptr)
    {
      // If button pressed
      if(btn.state == true)
      {
        // Press button on the screen
        ui_btn->SetPressed(true);
      }
      else // Released
      {
        // Release button on the screen
        ui_btn->SetPressed(false);
        // And call callback
        ths.ProcessCallback(ui_btn);
      }
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}
