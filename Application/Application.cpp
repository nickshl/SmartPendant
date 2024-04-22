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
// ***   Get Instance   ********************************************************
// *****************************************************************************
Application& Application::GetInstance(void)
{
   static Application application;
   return application;
}

// *****************************************************************************
// ***   Application Setup   ***************************************************
// *****************************************************************************
Result Application::Setup()
{
  // Set display inversion if needed
  display_drv.InvertDisplay(NVM::GetInstance().GetDisplayInvert());
  // Set display rotation
  display_drv.SetRotation((IDisplay::Rotation)rotation);
  // Set display background color
  display_drv.SetBackgroundColor(COLOR_DARKGREY);

  status_box.SetParams(0, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3 - Font_12x16::GetInstance().GetCharH() * 2 - 2, display_drv.GetScreenW() - Font_12x16::GetInstance().GetCharW() * 6, Font_12x16::GetInstance().GetCharH() * 2, COLOR_GREY, false);
  state_str.SetParams("", 4, status_box.GetStartY() + status_box.GetHeight() / 2 - Font_12x16::GetInstance().GetCharH() / 2, COLOR_WHITE, Font_12x16::GetInstance());
  status_str.SetParams("", 8 * Font_12x16::GetInstance().GetCharW(), state_str.GetEndY() -Font_8x12::GetInstance().GetCharH(), COLOR_WHITE, Font_8x12::GetInstance());
  status_box.Show(1000);
  state_str.Show(1001);
  status_str.Show(1001);

  // Release control by default at startup
  GrblComm::GetInstance().ReleaseControl();
  // MPG button
  mpg_btn.SetParams("MPG", display_drv.GetScreenW() - Font_12x16::GetInstance().GetCharW() * 6 - 1, status_box.GetStartY(), Font_12x16::GetInstance().GetCharW() * 6, status_box.GetHeight(), true);
  mpg_btn.SetCallback(this);
  mpg_btn.SetPressed(false);
  mpg_btn.Show(1000);

  // Pages for screens(first call to set parameters)
  header.SetParams(0, 0, display_drv.GetScreenW(), 40, Header::MAX_PAGES);
  // Screens & Captions
  header.SetText(scr_cnt, "MPG", Font_12x16::GetInstance());
  header.SetImage(scr_cnt, MPG);
  scr[scr_cnt++] = &DirectControlScr::GetInstance();
  header.SetText(scr_cnt, "OVERRIDE", Font_12x16::GetInstance());
  scr[scr_cnt++] = &OverrideCtrlScr::GetInstance();
  header.SetText(scr_cnt, "POWER FEED", Font_12x16::GetInstance());
  scr[scr_cnt++] = &DelayControlScr::GetInstance();
  header.SetText(scr_cnt, "ROTARY TABLE", Font_12x16::GetInstance());
  header.SetImage(scr_cnt, RotaryTable);
  scr[scr_cnt++] = &RotaryTableScr::GetInstance();
  header.SetText(scr_cnt, "SD CARD", Font_12x16::GetInstance());
  scr[scr_cnt++] = &ProgrammSender::GetInstance();
  header.SetText(scr_cnt, "PROBE", Font_12x16::GetInstance());
  scr[scr_cnt++] = &ProbeScr::GetInstance();
  header.SetText(scr_cnt, "SETTINGS", Font_12x16::GetInstance());
  scr[scr_cnt++] = &SettingsScr::GetInstance();
  // Pages for screens(second call to resize to actual numbers of pages)
  header.SetParams(0, 0, display_drv.GetScreenW(), 40, scr_cnt);
  // Set callback
  header.SetCallback(this);
  header.Show(2000);

  // Setup all screens
  for(uint32_t i = 0u; i < scr_cnt; i++)
  {
    scr[i]->Setup(40, display_drv.GetScreenH() - 40 - status_box.GetHeight());
  }

  // Set index
  scr_idx = 0u;
  // Show first screen
  scr[scr_idx]->Show();

  // Set callback handler for left and right buttons
  input_drv.AddButtonsCallbackHandler(this, reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this, InputDrv::BTNM_USR, btn_cble);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result Application::TimerExpired()
{
  // Update state & status
  state_str.SetString(GrblComm::GetInstance().GetCurrentStateName());
  status_str.SetString(GrblComm::GetInstance().GetCurrentStatusName());

  // If button pressed - we requested control
  if(mpg_btn.GetPressed())
  {
    // If we gain control - change color to green, if not - change color to red
    mpg_btn.SetColor(GrblComm::GetInstance().GetMpgMode() ? COLOR_GREEN : COLOR_RED);
  }
  else // If button unpressed - we released control
  {
    // If we don't have control - change color to white, if still have it - change color to red
    mpg_btn.SetColor(GrblComm::GetInstance().GetMpgMode() ? COLOR_RED : COLOR_WHITE);
  }

  // Process timer
  scr[scr_idx]->TimerExpired(TASK_TIMER_PERIOD_MS);

  // Update Display
  display_drv.UpdateDisplay();
  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProcessMessage function   *********************************************
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
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result Application::ProcessCallback(const void* ptr)
{
  // Check MPG button
  if(ptr == &mpg_btn)
  {
    if(GrblComm::GetInstance().GetMpgModeRequest() == true)
    {
      GrblComm::GetInstance().ReleaseControl();
      mpg_btn.SetPressed(false);
    }
    else
    {
      GrblComm::GetInstance().GainControl();
      mpg_btn.SetPressed(true);
    }
  }
  else if(ptr == &header) // Change screen when another page selected
  {
    ChangeScreen(header.GetSelectedPage());
  }
  else // Process screen callback if needed
  {
    scr[scr_idx]->ProcessCallback(ptr);
  }

  // Always good
  return Result::RESULT_OK;
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

    // Act on press button only
    if(btn.state == true)
    {
      if(btn.btn == InputDrv::BTN_LEFT_UP)
      {
        if(ths.scr_idx > 0u) ths.ChangeScreen(ths.scr_idx - 1u);
      }
      else if(btn.btn == InputDrv::BTN_RIGHT_UP)
      {
        if(ths.scr_idx < ths.scr_cnt - 1u) ths.ChangeScreen(ths.scr_idx + 1u);
      }
      else if(btn.btn == InputDrv::BTN_USR)
      {
//        // !!! TEST ONLY !!!
//        rotation += 1u;
//        if(rotation >= IDisplay::ROTATION_CNT) rotation = IDisplay::ROTATION_TOP;
//        Setup();
//        // !!! TEST ONLY !!!*/

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
