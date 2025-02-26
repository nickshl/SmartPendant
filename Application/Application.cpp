//******************************************************************************
//  @file Application.cpp
//  @author Nicolai Shlapunov
//
//  @details Application: User Application Class, implementation
//
//  @copyright Copyright (c) 2023, Devtronic & Nicolai Shlapunov
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
#include "Application.h"

// *****************************************************************************
// ***   Public: Get Instance   ************************************************
// *****************************************************************************
Application& Application::GetInstance(void)
{
   static Application application;
   return application;
}

// *****************************************************************************
// ***   Public: Application Setup   *******************************************
// *****************************************************************************
Result Application::Setup()
{
  // Read data from chip to memory
  NVM::GetInstance().ReadData();

  // Set display inversion if needed
  display_drv.InvertDisplay(NVM::GetInstance().GetDisplayInvert());
  // Set display rotation
  display_drv.SetRotation((IDisplay::Rotation)rotation);
  // Set display background color
  display_drv.SetBackgroundColor(COLOR_DARKGREY);

  // Box for status
  status_box.SetParams(0, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3 - Font_12x16::GetInstance().GetCharH() * 2 - 2, display_drv.GetScreenW() - Font_12x16::GetInstance().GetCharW() * 6, Font_12x16::GetInstance().GetCharH() * 2, COLOR_GREY, false);
  status_box.Show(1000);

  // Release control by default at startup
  grbl_comm.ReleaseControl();
  // MPG button
  mpg_btn.SetParams("MPG", display_drv.GetScreenW() - Font_12x16::GetInstance().GetCharW() * 6 - 1, status_box.GetStartY(), Font_12x16::GetInstance().GetCharW() * 6, status_box.GetHeight(), true);
  mpg_btn.SetCallback(this);
  mpg_btn.SetPressed(false);
  mpg_btn.Show(1000);

  // Info strings
  state_str.SetParams("", 4, status_box.GetStartY() + status_box.GetHeight() / 2 - Font_12x16::GetInstance().GetCharH() / 2, COLOR_WHITE, Font_12x16::GetInstance());
  status_str.SetParams("", state_str.GetStartX() + 5 * Font_12x16::GetInstance().GetCharW() + Font_8x12::GetInstance().GetCharW(), status_box.GetEndY() - Font_8x12::GetInstance().GetCharH() - 2, COLOR_WHITE, Font_8x12::GetInstance());
  pins_str.SetParams("", mpg_btn.GetStartX() - Font_8x8::GetInstance().GetCharW() * 11u, status_box.GetStartY() + 4, COLOR_RED, Font_8x8::GetInstance());
  state_str.Show(1001);
  status_str.Show(1002);
  pins_str.Show(1003);

  // Set Soft Buttons parameters
  InitSoftButtons();

  // Initialize header
  InitHeader();

  // Setup all screens
  for(uint32_t i = 0u; i < scr_cnt; i++)
  {
    scr[i]->Setup(GetScreenStartY(), status_box.GetStartY() - GetScreenStartY());
  }

  // Set index
  scr_idx = 0u;
  // Show first screen
  scr[scr_idx]->Show();

  // Set callback handler for left and right buttons
  input_drv.AddButtonsCallbackHandler(this, reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this, InputDrv::BTNM_USR | InputDrv::BTNM_LEFT | InputDrv::BTNM_RIGHT, btn_cble);

  // Init axis data windows
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].SetNumber(0);
  }

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Public: TimerExpired function   ***************************************
// *****************************************************************************
Result Application::TimerExpired()
{
  // Update state & status
  state_str.SetString(grbl_comm.GetCurrentStateName());
  status_str.SetString(grbl_comm.GetCurrentStatusName());
  pins_str.SetString(grbl_comm.GetPinsStr(), grbl_comm.IsPinsStrChanged());

  // Update numbers with current position and position difference
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].SetNumber(grbl_comm.GetAxisPosition(i));
  }

  // If button pressed - we requested control
  if(mpg_btn.GetPressed())
  {
    // If we gain control - change color to green, if not - change color to red
    mpg_btn.SetColor(grbl_comm.GetMpgMode() ? COLOR_GREEN : COLOR_RED);
  }
  else // If button unpressed - we released control
  {
    // If we don't have control - change color to white, if still have it - change color to red
    mpg_btn.SetColor(grbl_comm.GetMpgMode() ? COLOR_RED : COLOR_WHITE);
  }

  // If controller settings changed, we have to setup screens again to proper
  // handling of Metric/Imperial or Mill/Lathe change
  if(grbl_comm.IsSettingsChanged())
  {
    // Hide screen
    scr[scr_idx]->Hide();
    // Initialize header
    InitHeader();
    // Setup all screens
    for(uint32_t i = 0u; i < scr_cnt; i++)
    {
      scr[i]->Setup(40, status_box.GetStartY() - 40);
    }
    // Set index
    scr_idx = 0u;
    // Show first screen
    scr[scr_idx]->Show();
  }

  // Call timer callback for current screen
  scr[scr_idx]->TimerExpired(TASK_TIMER_PERIOD_MS);

  // Update Display
  display_drv.UpdateDisplay();

  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Public: ProcessMessage function   *************************************
// *****************************************************************************
Result Application::ProcessMessage()
{
  Result result = Result::ERR_NULL_PTR;

  switch(rcv_msg.type)
  {
    case TASK_CALLBACK_MSG:
      // Result ok
      result = Result::RESULT_OK;
      break;

    default:
      result = Result::ERR_INVALID_ITEM;
      break;
  }

  return result;
}

// *****************************************************************************
// ***   Public: ProcessCallback function   ************************************
// *****************************************************************************
Result Application::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  // Check MPG button
  if(ptr == &mpg_btn)
  {
    if(grbl_comm.GetMpgModeRequest() == true)
    {
      grbl_comm.ReleaseControl();
      mpg_btn.SetPressed(false);
    }
    else
    {
      grbl_comm.GainControl();
      mpg_btn.SetPressed(true);
    }
  }
  else if(ptr == &header) // Change screen when another page selected
  {
    ChangeScreen(header.GetSelectedPage());
  }
  else // Process screen callback if it is something else
  {
    result = scr[scr_idx]->ProcessCallback(ptr);
  }

  // If request wasn't handled by screen
  if(result == Result::ERR_UNHANDLED_REQUEST)
  {
    if(ptr == &left_btn)
    {
      // If we already run program
      if(grbl_comm.GetState() != GrblComm::RUN)
      {
        grbl_comm.Run(); // Send Run command for continue
      }
      else
      {
        grbl_comm.Hold(); // Send Hold command for pause
      }
    }
    else if(ptr == &right_btn)
    {
      if(grbl_comm.GetState() == GrblComm::ALARM)
      {
        if(grbl_comm.GetStatusCode() == GrblComm::Status_NotAllowedCriticalEvent)
        {
          grbl_comm.Reset(); // Send Reset command
        }
        else
        {
          grbl_comm.Unlock(); // Send Unlock command
        }
      }
      else if((grbl_comm.GetState() == GrblComm::UNKNOWN) || (grbl_comm.GetState() == GrblComm::HOME))
      {
        grbl_comm.Reset(); // Send Reset command
      }
      else
      {
        grbl_comm.Stop(); // Send Stop command
      }
    }
    else; //Do nothing - MISRA rule
  }

  // Always good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Public: InitSoftButtons function   ************************************
// *****************************************************************************
void Application::InitSoftButtons(bool three_buttons)
{
  // Width depends how many buttons we have
  int32_t btn_width = three_buttons ? ((display_drv.GetScreenW() - BORDER_W * 2) / 3) : (display_drv.GetScreenW() / 2 - BORDER_W);

  // Left soft button
  left_btn.SetParams("", 0, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3, btn_width, Font_8x12::GetInstance().GetCharH() * 3, true);
  left_btn.SetCallback(AppTask::GetCurrent());
  // Middle button
  if(three_buttons)
  {
    middle_btn.SetParams("", left_btn.GetEndX() + BORDER_W + 1, left_btn.GetStartY(), btn_width, left_btn.GetHeight(), true);
    middle_btn.SetCallback(AppTask::GetCurrent());
  }
  // Right soft button
  right_btn.SetParams("", display_drv.GetScreenW() - btn_width, left_btn.GetStartY(), btn_width, left_btn.GetHeight(), true);
  right_btn.SetCallback(AppTask::GetCurrent());
}

// *****************************************************************************
// ***   Public: UpdateLeftButtonText function   *******************************
// *****************************************************************************
void Application::UpdateLeftButtonText()
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
}

// *****************************************************************************
// ***   Public: UpdateLeftButtonIdleText function   ***************************
// *****************************************************************************
void Application::UpdateLeftButtonIdleText(const char* str)
{
  // Update left button text
  if(((grbl_comm.GetState() == GrblComm::IDLE) || (grbl_comm.GetState() == GrblComm::UNKNOWN)) && (str != nullptr))
  {
    left_btn.SetString(str);
  }
  else
  {
    UpdateLeftButtonText();
  }
}

// *****************************************************************************
// ***   Public: UpdateRightButtonText function   ******************************
// *****************************************************************************
void Application::UpdateRightButtonText()
{
  // Update right button text
  if(grbl_comm.GetState() == GrblComm::ALARM)
  {
    if(grbl_comm.GetStatusCode() == GrblComm::Status_NotAllowedCriticalEvent)
    {
      right_btn.SetString("Reset");
    }
    else
    {
      right_btn.SetString("Unlock");
    }
  }
  else if((grbl_comm.GetState() == GrblComm::UNKNOWN) || (grbl_comm.GetState() == GrblComm::HOME))
  {
    right_btn.SetString("Reset");
  }
  else
  {
    right_btn.SetString("Stop");
  }
}

// *****************************************************************************
// ***   Public: UpdateRightButtonIdleText function   **************************
// *****************************************************************************
void Application::UpdateRightButtonIdleText(const char* str)
{
  // Update right button text
  if((grbl_comm.GetState() == GrblComm::IDLE) && (str != nullptr))
  {
    right_btn.SetString(str);
  }
  else
  {
    UpdateRightButtonText();
  }
}

// *****************************************************************************
// ***   Public: ChangeScreen function   ***************************************
// *****************************************************************************
void Application::ChangeScreen(IScreen& screen)
{
  // Find new index
  for(uint32_t i = 0u; i < NumberOf(scr); i++)
  {
    if(scr[i] == &screen)
    {
      // Hide old screen
      scr[scr_idx]->Hide();
      // Save scale to control
      scr_idx = i;
      // Set new page
      header.SetSelectedPage(scr_idx);
      // Show new screen
      scr[scr_idx]->Show();
      // Break the cycle
      break;
    }
  }
}

// *****************************************************************************
// ***   Private: ProcessButtonCallback function   *****************************
// *****************************************************************************
Result Application::ProcessButtonCallback(Application* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    Application& ths = *obj_ptr;
    // Get pressed button
    InputDrv::ButtonCallbackData btn = *((InputDrv::ButtonCallbackData*)ptr);

    // *** Left/Right buttons **************************************************

    // UI Button pointer
    UiButton *ui_btn = nullptr;

    // Find button pointer
    if(btn.btn == InputDrv::BTN_LEFT)       ui_btn = &ths.left_btn;
    else if(btn.btn == InputDrv::BTN_RIGHT) ui_btn = &ths.right_btn;
    else; // Do nothing - MISRA rule

    // If button object found, It on display and it active
    if((ui_btn != nullptr) && ui_btn->IsShow() && ui_btn->IsActive())
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

    // *** Other buttons *******************************************************

    // Act on press button only
    if(btn.state == true)
    {
      if(btn.btn == InputDrv::BTN_USR)
      {
        if(GrblComm::GetInstance().GetMpgModeRequest() == true)
        {
          GrblComm::GetInstance().ReleaseControl();
          ths.mpg_btn.SetPressed(false);
        }
        else
        {
          GrblComm::GetInstance().GainControl();
          ths.mpg_btn.SetPressed(true);
        }
      }
      else
      {
        ; // Do nothing - MISRA rule
      }
    }
    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ChangeScreen function   **************************************
// *****************************************************************************
void Application::ChangeScreen(uint8_t scrn)
{
  // Hide old screen
  scr[scr_idx]->Hide();
  // Save scale to control
  scr_idx = scrn;
  // Set new page
  header.SetSelectedPage(scr_idx);
  // Show new screen
  scr[scr_idx]->Show();
}

// *****************************************************************************
// ***   Private: InitHeader function   ****************************************
// *****************************************************************************
void Application::InitHeader()
{
  // Hide header
  header.Hide();
  // Pages for screens(first call to set parameters)
  header.SetParams(0, 0, display_drv.GetScreenW(), 40, Header::MAX_PAGES);
  // Screens & Captions
  scr_cnt = 0u;
  header.SetText(scr_cnt, "MPG", Font_12x16::GetInstance());
  header.SetImage(scr_cnt, MPG);
  scr[scr_cnt++] = &DirectControlScr::GetInstance();
  header.SetText(scr_cnt, "OVERRIDE", Font_12x16::GetInstance());
  scr[scr_cnt++] = &OverrideCtrlScr::GetInstance();
  header.SetText(scr_cnt, "POWER FEED", Font_12x16::GetInstance());
  scr[scr_cnt++] = &DelayControlScr::GetInstance();
  // Rotary Table available only for mill
  if(grbl_comm.GetModeOfOperation() == GrblComm::MODE_OF_OPERATION_MILL)
  {
    header.SetText(scr_cnt, "ROTARY TABLE", Font_12x16::GetInstance());
//    header.SetImage(scr_cnt, RotaryTable);
    scr[scr_cnt++] = &RotaryTableScr::GetInstance();
  }
  header.SetText(scr_cnt, "GCODE SENDER", Font_12x16::GetInstance());
  scr[scr_cnt++] = &ProgramSender::GetInstance();
  // Probing available only for mill
  if(grbl_comm.GetModeOfOperation() == GrblComm::MODE_OF_OPERATION_MILL)
  {
    header.SetText(scr_cnt, "PROBE", Font_12x16::GetInstance());
    scr[scr_cnt++] = &ProbeScr::GetInstance();
  }
  // Milling Operation available only for mill
  if(grbl_comm.GetModeOfOperation() == GrblComm::MODE_OF_OPERATION_MILL)
  {
    header.SetText(scr_cnt, "MILLING OPERATIONS", Font_12x16::GetInstance());
    scr[scr_cnt++] = &MillOpsScr::GetInstance();
  }
  // Turning Operation available only for lathe
  if(grbl_comm.GetModeOfOperation() == GrblComm::MODE_OF_OPERATION_LATHE)
  {
    header.SetText(scr_cnt, "TURNING OPERATIONS", Font_12x16::GetInstance());
    scr[scr_cnt++] = &LatheOpsScr::GetInstance();
  }
  header.SetText(scr_cnt, "SETTINGS", Font_12x16::GetInstance());
  scr[scr_cnt++] = &SettingsScr::GetInstance();
  // Pages for screens(second call to resize to actual numbers of pages)
  header.SetParams(0, 0, display_drv.GetScreenW(), 40, scr_cnt);
  // Resize buttons to occupy all available space
  header.ResizeButtons();
  // Set callback
  header.SetCallback(this);
  header.Show(2000);
}
