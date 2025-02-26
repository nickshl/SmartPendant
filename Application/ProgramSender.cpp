//******************************************************************************
//  @file ProgramSender.cpp
//  @author Nicolai Shlapunov
//
//  @details ProgramSender: User ProgramSender Class, implementation
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
#include <ProgramSender.h>
#include "Application.h"

#include "fatfs.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
ProgramSender& ProgramSender::GetInstance()
{
  static ProgramSender pgmsenderscr;
  return pgmsenderscr;
}

// *****************************************************************************
// ***   ProgramSender Setup   *************************************************
// *****************************************************************************
Result ProgramSender::Setup(int32_t y, int32_t height)
{
  constexpr int32_t CTRL_HEIGHT = 40;

  // Initialize font
  mem_info.SetParams(mem_info_buf, 0, 0, COLOR_WHITE, Font_6x8::GetInstance());

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
  text_box.Setup(0, y, display_drv.GetScreenW(), height - Font_8x12::GetInstance().GetCharH() * 2u - BORDER_W*2 - CTRL_HEIGHT);

  // Feed override
  feed_dw.SetParams(BORDER_W, y + height - Font_8x12::GetInstance().GetCharH() * 2u - BORDER_W - BORDER_W - CTRL_HEIGHT, (display_drv.GetScreenW() - 2*CTRL_HEIGHT - 3*BORDER_W) / 2, CTRL_HEIGHT, 5u, 0u);
  feed_dw.SetBorder(BORDER_W, COLOR_DARKBLUE);
  feed_dw.SetDataFont(Font_12x16::GetInstance());
  feed_dw.SetNumber(0);
  feed_dw.SetUnits("%", DataWindow::RIGHT);
  feed_dw.SetCallback(AppTask::GetCurrent());
  feed_name.SetParams("FEED", feed_dw.GetStartX() + BORDER_W*3/2, feed_dw.GetStartY() + BORDER_W*3/2, COLOR_WHITE, Font_6x8::GetInstance());
  // Speed override
  speed_dw.SetParams(feed_dw.GetEndX() + BORDER_W, feed_dw.GetStartY(), feed_dw.GetWidth(), feed_dw.GetHeight(), 5u, 0u);
  speed_dw.SetBorder(BORDER_W, COLOR_DARKBLUE);
  speed_dw.SetDataFont(Font_12x16::GetInstance());
  speed_dw.SetNumber(0);
  speed_dw.SetUnits("%", DataWindow::RIGHT);
  speed_dw.SetCallback(AppTask::GetCurrent());
  speed_name.SetParams("SPEED", speed_dw.GetStartX() + BORDER_W*3/2, speed_dw.GetStartY() + BORDER_W*3/2, COLOR_WHITE, Font_6x8::GetInstance());
  // Buttons for control flood coolant
  flood_btn.SetParams("F", speed_dw.GetEndX() + BORDER_W, speed_dw.GetStartY(), speed_dw.GetHeight(), speed_dw.GetHeight(), true);
  flood_btn.SetFont(Font_12x16::GetInstance());
  flood_btn.SetCallback(AppTask::GetCurrent());
  flood_btn.Disable();
  // Buttons for control mist coolant
  mist_btn.SetParams("M", flood_btn.GetEndX() + BORDER_W, speed_dw.GetStartY(), speed_dw.GetHeight(), speed_dw.GetHeight(), true);
  mist_btn.SetFont(Font_12x16::GetInstance());
  mist_btn.SetCallback(AppTask::GetCurrent());
  mist_btn.Disable();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result ProgramSender::Show()
{
  // Set encoder callback handler(before menu show since menu will handle it also)
  InputDrv::GetInstance().AddEncoderCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessEncoderCallback), this, enc_cble);

  // Show free memory info
  UpdateMemoryInfo();
  mem_info.Show(10000);

  // Update text - in case it is generated, we have to count lines
  text_box.SetText(p_text);
  // Show text box
  text_box.Show(100);

  // Axis data
  for(uint32_t i = 0u; i < grbl_comm.GetLimitedNumberOfAxis(3u); i++)
  {
    DataWindow& dw_real = Application::GetInstance().GetRealDataWindow(i);
    String& dw_real_name = Application::GetInstance().GetRealDataWindowNameString(i);

    // Real position
    dw_real.SetParams(BORDER_W + ((display_drv.GetScreenW() - BORDER_W * 4) / 3 + BORDER_W) * i, feed_dw.GetEndY() + BORDER_W, (display_drv.GetScreenW() - BORDER_W * 4) / 3, Font_8x12::GetInstance().GetCharH() * 2u, 8u, grbl_comm.GetUnitsPrecision());
    dw_real.SetBorder(BORDER_W / 2, COLOR_GREY);
    dw_real.SetDataFont(Font_8x12::GetInstance());
    dw_real.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT, Font_6x8::GetInstance());
    // Axis Name
    dw_real_name.SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_6x8::GetInstance());
    dw_real_name.Move(dw_real.GetStartX() + BORDER_W, dw_real.GetStartY() + BORDER_W);

    dw_real.Show(100);
    dw_real_name.Show(100);
  }

  // Reinit all three Soft Buttons
  Application::GetInstance().InitSoftButtons(true);

  // Run button
  left_btn.SetString("Run");
  left_btn.Show(102);
  // Open button
  middle_btn.SetString("Open");
  middle_btn.Show(102);
  // Stop button
  right_btn.SetString("Stop");
  right_btn.Show(102);

  // Feed objects
  feed_dw.Show(100);
  feed_name.Show(100);
  // Speed objects
  speed_dw.Show(100);
  speed_name.Show(100);
  // Coolant buttons
  flood_btn.Show(100);
  mist_btn.Show(100);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result ProgramSender::Hide()
{
  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);

  // Hide free memory info
  mem_info.Hide();

  // Hide menu
  menu.Hide();
  // Hide text box
  text_box.Hide();

  // We may have file open, close it and clear text box
  if(p_text == nullptr)
  {
    f_close(&SDFile);
  }

  // Axis data
  for(uint32_t i = 0u; i < GrblComm::AXIS_CNT; i++)
  {
    Application::GetInstance().GetRealDataWindow(i).Hide();
    Application::GetInstance().GetRealDataWindowNameString(i).Hide();
  }

  // Go button
  left_btn.Hide();
  // Open button
  middle_btn.Hide();
  // Reset button
  right_btn.Hide();

  // Feed objects
  feed_dw.Hide();
  feed_name.Hide();
  // Speed objects
  speed_dw.Hide();
  speed_name.Hide();
  // Coolant buttons
  flood_btn.Hide();
  mist_btn.Hide();

  // Reinit Soft Buttons to change their size back
  Application::GetInstance().InitSoftButtons(false);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result ProgramSender::TimerExpired(uint32_t interval)
{
  // Update left & right button text
  Application::GetInstance().UpdateLeftButtonText();
  Application::GetInstance().UpdateRightButtonText();

  // Update numbers with current overrides
  feed_dw.SetNumber(grbl_comm.GetFeedOverride());
  speed_dw.SetNumber(grbl_comm.GetSpeedOverride());
  // Set coolant state
  flood_btn.SetColor(grbl_comm.GetCoolantFlood() ? COLOR_GREEN : COLOR_WHITE);
  mist_btn.SetColor(grbl_comm.GetCoolantMist() ? COLOR_GREEN : COLOR_WHITE);

  if(run)
  {
    // Process speed & feed change
    ProcessSpeedFeed();

    // We should stream program if state is Idle, Run or Hold and we in control
    if(((grbl_comm.GetState() == GrblComm::IDLE) || (grbl_comm.GetState() == GrblComm::RUN) || (grbl_comm.GetState() == GrblComm::HOLD)) && (grbl_comm.IsInControl()))
    {
      // If we finished streaming
      if(finished)
      {
        // Wait until IDLE state
        if(grbl_comm.GetState() == GrblComm::IDLE)
        {
          // Then clear run flag
          run = false;
        }
      }
      else
      {
        // If ID is zero - we didn't send any commands yet
        GrblComm::status_t result = (id != 0u) ? grbl_comm.GetCmdResult(id) : GrblComm::Status_OK;
        // If result of previous command is ok
        if((result == GrblComm::Status_OK) || (result == GrblComm::Status_Next_Cmd_Executed))
        {
          // Buffer for command
          char cmd[128u];
          // Since all program commands have striped out CR and LF, we have to add it
          snprintf(cmd, NumberOf(cmd), "%s\r", text_box.GetSelectedStringText());
          // Send new command
          if(grbl_comm.SendCmd(cmd, id) == Result::RESULT_OK)
          {
            int32_t select = text_box.GetSelect();
            int32_t scroll = text_box.GetScroll();
            // Load next string for SD streamed programs
            if(p_text == nullptr)
            {
              // If we did not passed half the screen or if file closed
              // and we need to finish remaining lines
              if((select < text_box.GetNumberOfVisibleLines() / 2) || f_eof(&SDFile))
              {
                // Go to next line
                text_box.Select(select + 1);
                // Can't go further - end of program
                if(select == text_box.GetSelect())
                {
                  // Set finished flag
                  finished = true;
                }
              }
              else
              {
                // Buffer to read string 80 + 2 + 1
                char str[128] = {0};
                // Read line from file
                if(f_gets(str, NumberOf(str), &SDFile) != nullptr)
                {
                  // Null-terminate just in case
                  str[NumberOf(str) - 1] = '\0';
                  // If we read line longer than 80 characters + possible CR & LF characters
                  if(strlen(str) > 80 + 2)
                  {
                    // Rewind to the end of file
                    f_lseek(&SDFile, SDFile.obj.objsize);
                  }
                  else
                  {
                    // Set this line to text_box
                    text_box.AddLine(str);
                  }
                }
              }
            }
            else
            {
              // If we half past screen
              if(select - scroll >= text_box.GetNumberOfVisibleLines() / 2)
              {
                // Scroll to to see next lines to see what will send next
                text_box.Scroll(scroll + 1);
              }
              // Go to next line
              text_box.Select(select + 1);
              // Can't go further - end of program
              if(select == text_box.GetSelect())
              {
                // Set finished flag
                finished = true;
              }
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
    }
    else
    {
      // In case of any unexpected error - stop the program
      run = false;
    }
  }
  else if(grbl_comm.GetState() == GrblComm::RUN) // If we finished program, but controller still running
  {
    // Process speed & feed change
    ProcessSpeedFeed();
  }
  else
  {
    // Safety measure: allow run program only from the beginning
    // TODO: add dialog box "Are you sure you want to run program from current position?" instead
    if(text_box.GetSelect() == 0)
    {
      left_btn.Enable();
    }
    else
    {
      left_btn.Disable();
    }
    // If feed control enabled - disable it
    if(feed_dw.IsActive())
    {
      feed_dw.SetActive(false);
      feed_dw.SetBorder(BORDER_W, COLOR_DARKBLUE);
      feed_dw.SetSelected(false);
    }
    // If speed control enabled - disable it
    if(speed_dw.IsActive())
    {
      speed_dw.SetActive(false);
      speed_dw.SetBorder(BORDER_W, COLOR_DARKBLUE);
      speed_dw.SetSelected(false);
    }
    // Disable coolant control
    flood_btn.Disable();
    mist_btn.Disable();
    // Enable buttons back
    middle_btn.Enable();
    // Enable screen change if program isn't running
    Application::GetInstance().EnableScreenChange();
    // If encoder turned and we have program in memory
    if((enc_val != 0) && (p_text != nullptr))
    {
      // Select line
      text_box.Select(text_box.GetSelect() + enc_val);
      //text_box.Scroll(text_box.GetScroll() + enc_val);
      // Clear encoder value
      enc_val = 0;
    }
  }

  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *************************************************************************
// ***   Private: UpdateMemoryInfo function   ******************************
// *************************************************************************
void ProgramSender::UpdateMemoryInfo()
{
  // Get memory stats from FreeRTOS
  HeapStats_t HeapStats;
  vPortGetHeapStats(&HeapStats);
  snprintf(mem_info_buf, sizeof(mem_info_buf), "Available memory: %db", HeapStats.xSizeOfLargestFreeBlockInBytes);
  // Update string and recalculate size
  mem_info.SetString(mem_info_buf, true);
  // Set string params
  mem_info.SetParams(mem_info_buf, display_drv.GetScreenW()/2 - mem_info.GetWidth()/2, 30, COLOR_WHITE, Font_6x8::GetInstance());
}

// *************************************************************************
// ***   Private: ProcessSpeedFeed function   ******************************
// *************************************************************************
Result ProgramSender::ProcessSpeedFeed()
{
  Result result = Result::RESULT_OK;

  // Update feed if necessary. One step at a timer tick.
  if(feed_val > 0)
  {
    if(feed_val > 10)
    {
      result = grbl_comm.FeedCoarsePlus();
      feed_val -= 10;
    }
    else
    {
      result = grbl_comm.FeedFinePlus();
      feed_val--;
    }
  }
  else if (feed_val < 0)
  {
    if(feed_val < -10)
    {
      result = grbl_comm.FeedCoarseMinus();
      feed_val += 10;
    }
    else
    {
      result = grbl_comm.FeedFineMinus();
      feed_val++;
    }
  }
  else
  {
    ; // Do nothing
  }

  // Update speed if necessary. One step at a timer tick.
  if(speed_val > 0)
  {
    if(speed_val > 10)
    {
      result = grbl_comm.SpeedCoarsePlus();
      speed_val -= 10;
    }
    else
    {
      result = grbl_comm.SpeedFinePlus();
      speed_val--;
    }
  }
  else if (speed_val < 0)
  {
    if(speed_val < -10)
    {
      result = grbl_comm.SpeedCoarseMinus();
      speed_val += 10;
    }
    else
    {
      result = grbl_comm.SpeedFineMinus();
      speed_val++;
    }
  }
  else
  {
    ; // Do nothing
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ProcessMenuOkCallback function   *****************************
// *****************************************************************************
Result ProgramSender::ProcessMenuOkCallback(ProgramSender* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ProgramSender& ths = *obj_ptr;

    // Hide the menu
    ths.menu.Hide();

    // Open file
    FRESULT fres = f_open(&SDFile, ths.menu_items[(uint32_t)ptr].text, FA_OPEN_EXISTING | FA_READ);
    // Write data to file
    if(fres == FR_OK)
    {
      // Get file size
      uint32_t fsize = f_size(&SDFile) + 1u;
      // Allocate memory for data
      ths.AllocateDataBuffer(fsize);
      // Check if allocation was successful
      if(ths.p_text != nullptr)
      {
        // Read bytes
        UINT wbytes = 0u;
        // Read text
        fres = f_read(&SDFile, ths.p_text, fsize, &wbytes);
        // And null-terminator to it
        ths.p_text[wbytes] = 0x00;
        // Set text to text box
        if(!ths.text_box.SetText(ths.p_text))
        {
          // If program contains lines longer than 80 characters - show message
          ths.text_box.SetText("; Program contain lines longer\n\r; than 80 characters");
        }
        // Close file
        fres = f_close(&SDFile);
      }
      else
      {
        // Clear text buffer to switch into line mode
        ths.text_box.SetText(nullptr);

        // Buffer to read string
        char str[128] = {0};
        // Fill all visible lines
        for(int32_t i = 0; i < ths.text_box.GetNumberOfVisibleLines(); i++)
        {
          // Read line from file
          f_gets(str, NumberOf(str), &SDFile);
          // Null-terminate just in case
          str[NumberOf(str) - 1] = '\0';
          // If we read line longer than 80 characters + possible CR & LF characters
          if(strlen(str) > 80 + 2)
          {
            // Close file - we can't continue
            f_close(&SDFile);
            // If beginning of program contains lines longer than 80 characters - show message
            ths.text_box.SetText("; Program contain lines longer\n\r; than 80 characters");
            // Break the cycle
            break;
          }
          else
          {
            // Set this line to text_box
            ths.text_box.AddLine(str);
          }
        }
      }
    }
    else
    {
      // If memory allocation operation isn't successful set text
      ths.text_box.SetText("; Error open file!");
    }

    // And show it
    ths.text_box.Show(100);
    // Left button
    ths.left_btn.Show(102);
    // Open button
    ths.middle_btn.Show(102);
    // Right button
    ths.right_btn.Show(102);

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ProcessMenuCancelCallback function   *************************
// *****************************************************************************
Result ProgramSender::ProcessMenuCancelCallback(ProgramSender* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ProgramSender& ths = *obj_ptr;

    // Hide the menu
    ths.menu.Hide();
    // Set cancel text
    ths.text_box.SetText("; Cancel pressed in open dialog");
    // And show textbox
    ths.text_box.Show(100);
    // Run button
    ths.left_btn.Show(102);
    // Open button
    ths.middle_btn.Show(102);
    // Stop button
    ths.right_btn.Show(102);

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result ProgramSender::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  // Process Run button. Since we can call this handler after press of physical
  // button, we have to check if Run button is active.
  if(ptr == &left_btn)
  {
    // We should run program only if it doesn't already run, we in control and state is Idle
    if(!run && grbl_comm.IsInControl() && (grbl_comm.GetState() == GrblComm::IDLE))
    {
      // Clear id to run program
      id = 0u;
      // Set run flag to start program streaming
      run = true;
      finished = false;
      // Enable Feed & Speed control
      feed_dw.SetActive(true);
      speed_dw.SetActive(true);
      feed_dw.SetBorder(BORDER_W, COLOR_RED);
      speed_dw.SetBorder(BORDER_W, COLOR_RED);
      feed_dw.SetSelected(true);
      speed_dw.SetSelected(false);
      flood_btn.Enable();
      mist_btn.Enable();
      // Disable buttons while program is running
      middle_btn.Disable();
      // Disable screen change if program is running
      Application::GetInstance().DisableScreenChange();
    }
    else
    {
      result = Result::ERR_UNHANDLED_REQUEST; // For Application to handle it
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
  else if((ptr == &middle_btn) && (middle_btn.IsActive()))
  {
    // Stop timer to prevent queue overflow since SD card operations can take some
    // time.
    AppTask::GetCurrent()->StopTimer();

    // Clear text box
    text_box.SetText(nullptr);
    // We may have file open - close it
    f_close(&SDFile);
    // Clear current data to show available memory
    ReleaseDataPointer();

    // Reinit SD card
    BSP_SD_Init();

    // Mount SD
    FRESULT res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
    DIR dir;

    // Open the directory
    if(res == FR_OK)
    {
      res = f_opendir(&dir, "/");
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
          menu_items[idx].str.SetString(menu_items[idx].text, menu_items[idx].n, "%-12s%19lub", fno.fname, fno.fsize);
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
    // Open button
    middle_btn.Hide();
    // Reset button
    right_btn.Hide();

    // Set menu items count
    menu.SetCount(idx);
    // Show menu
    menu.Show(100);

    // Clear current position
    idx = 0u;
  }
  else if(ptr == &feed_dw)
  {
    speed_dw.SetSelected(false);
    feed_dw.SetSelected(true);
  }
  else if(ptr == &speed_dw)
  {
    feed_dw.SetSelected(false);
    speed_dw.SetSelected(true);
  }
  else if(ptr == &flood_btn)
  {
    grbl_comm.CoolantFloodToggle();
  }
  else if(ptr == &mist_btn)
  {
    grbl_comm.CoolantMistToggle();
  }
  else
  {
    ; // Do nothing - MISRA rule
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: AllocateDataBuffer   ******************************************
// *****************************************************************************
char* ProgramSender::AllocateDataBuffer(uint32_t size)
{
  // Always release data buffer before allocate it again
  ReleaseDataPointer();
  // Allocate memory for data
  p_text = new char[size];
  // If allocation is successful
  if(p_text != nullptr)
  {
    // Add null-terminator to the first element
    p_text[0] = '\0';
  }
  // Set buffer(or nullptr) to textbox
  text_box.SetText(p_text);
  // Update free memory info
  UpdateMemoryInfo();
  // Return result
  return p_text;
}

// *****************************************************************************
// ***   Public: ReleaseDataPointer   ******************************************
// *****************************************************************************
void ProgramSender::ReleaseDataPointer()
{
  // If buffer was previously allocated
  if(p_text != nullptr)
  {
    // Hide text box before delete buffer
    text_box.Hide();
    // Delete previously allocated buffer
    delete [] p_text;
    // Set text to nullptr
    p_text = nullptr;
  }
  // Set null data pointer
  text_box.SetText(nullptr);
  // Update free memory info
  UpdateMemoryInfo();
}

// *****************************************************************************
// ***   Private: ProcessEncoderCallback function   ****************************
// *****************************************************************************
Result ProgramSender::ProcessEncoderCallback(ProgramSender* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ProgramSender& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    // If program isn't running - scroll text
    if(!ths.run)
    {
      ths.enc_val += enc_val;
    }
    else if(ths.feed_dw.IsSelected())
    {
      ths.feed_val += enc_val;
    }
    else if(ths.speed_dw.IsSelected())
    {
      ths.speed_val += enc_val;
    }
    else
    {
      ; // Do nothing - MISRA rule
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private constructor   *************************************************
// *****************************************************************************
ProgramSender::ProgramSender() : left_btn(Application::GetInstance().GetLeftButton()),
                                 middle_btn(Application::GetInstance().GetMiddleButton()),
                                 right_btn(Application::GetInstance().GetRightButton()) {};
