//******************************************************************************
//  @file OverrideCtrlScr.cpp
//  @author Nicolai Shlapunov
//
//  @details OverrideCtrlScr: User OverrideCtrlScr Class, implementation
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
#include "OverrideCtrlScr.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
OverrideCtrlScr& OverrideCtrlScr::GetInstance()
{
  static OverrideCtrlScr directcontrolscr;
  return directcontrolscr;
}

// *****************************************************************************
// ***   OverrideCtrlScr Setup   ***********************************************
// *****************************************************************************
Result OverrideCtrlScr::Setup(int32_t y, int32_t height)
{
  int32_t start_y = y + BORDER_W;
  // Data window height
  uint32_t window_height = Font_8x12::GetInstance().GetCharH() * 5u;

  // Fill all windows
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    // Real position
    dw_real[i].SetParams(BORDER_W + ((display_drv.GetScreenW() - BORDER_W * 4) / 3 + BORDER_W) * i, start_y + BORDER_W*2 + Font_10x18::GetInstance().GetCharH(), (display_drv.GetScreenW() - BORDER_W * 4) / 3, (window_height - BORDER_W) / 2, 4u, 3u);
    dw_real[i].SetBorder(BORDER_W / 2, COLOR_GREY);
    dw_real[i].SetDataFont(Font_10x18::GetInstance());
    dw_real[i].SetNumber(0);
    dw_real[i].SetUnits("mm", DataWindow::RIGHT, Font_6x8::GetInstance());
    // Axis Name
    dw_real_name[i].SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_10x18::GetInstance());
    dw_real_name[i].Move(dw_real[i].GetStartX() + (dw_real[i].GetWidth() - dw_real_name[i].GetWidth()) / 2, dw_real[i].GetStartY() - BORDER_W - dw_real_name[i].GetHeight());
  }

  // Feed override
  feed_dw.SetParams(display_drv.GetScreenW() / 4, dw_real[0u].GetEndY() + BORDER_W*2, (display_drv.GetScreenW() - display_drv.GetScreenW() / 4 - BORDER_W) / 2,  window_height, 3u, 0u);
  feed_dw.SetBorder(BORDER_W, COLOR_RED);
  feed_dw.SetDataFont(Font_8x12::GetInstance(), 2u);
  feed_dw.SetNumber(0);
  feed_dw.SetUnits("%", DataWindow::RIGHT);
  feed_dw.SetCallback(AppTask::GetCurrent());
  feed_dw.SetActive(true);
  feed_name.SetParams("FEED:", 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
  feed_name.Move((feed_dw.GetStartX() / 2) - (feed_name.GetWidth() / 2), (feed_dw.GetStartY() + feed_dw.GetHeight() / 2) - (feed_name.GetHeight() / 2));
  // Set/Zero button
  feed_reset_btn.SetParams("100%", feed_dw.GetEndX() + BORDER_W, feed_dw.GetStartY(), display_drv.GetScreenW() - feed_dw.GetEndX() - BORDER_W * 2, feed_dw.GetHeight(), true);
  feed_reset_btn.SetCallback(AppTask::GetCurrent());

  // Speed override
  speed_dw.SetParams(feed_dw.GetStartX(), feed_dw.GetEndY() + BORDER_W*2, feed_dw.GetWidth(), feed_dw.GetHeight(), 3u, 0u);
  speed_dw.SetBorder(BORDER_W, COLOR_RED);
  speed_dw.SetDataFont(Font_8x12::GetInstance(), 2u);
  speed_dw.SetNumber(0);
  speed_dw.SetUnits("%", DataWindow::RIGHT);
  speed_dw.SetCallback(AppTask::GetCurrent());
  speed_dw.SetActive(true);
  speed_name.SetParams("SPEED:", 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
  speed_name.Move((speed_dw.GetStartX() / 2) - (speed_name.GetWidth() / 2), (speed_dw.GetStartY() + speed_dw.GetHeight() / 2) - (speed_name.GetHeight() / 2));
  // Set/Zero button
  speed_reset_btn.SetParams("100%", speed_dw.GetEndX() + BORDER_W, speed_dw.GetStartY(), display_drv.GetScreenW() - speed_dw.GetEndX() - BORDER_W * 2, speed_dw.GetHeight(), true);
  speed_reset_btn.SetCallback(AppTask::GetCurrent());


  // Buttons for control flood coolant
  flood_btn.SetParams("FLOOD", BORDER_W, speed_dw.GetEndY() + BORDER_W*2, display_drv.GetScreenW() / 2 - BORDER_W*2, speed_dw.GetHeight(), true);
  flood_btn.SetFont(Font_12x16::GetInstance());
  flood_btn.SetCallback(AppTask::GetCurrent());
  // Buttons for control mist coolant
  mist_btn.SetParams("MIST", display_drv.GetScreenW() / 2 + BORDER_W, speed_dw.GetEndY() + BORDER_W*2, display_drv.GetScreenW() / 2 - BORDER_W*2, speed_dw.GetHeight(), true);
  mist_btn.SetFont(Font_12x16::GetInstance());
  mist_btn.SetCallback(AppTask::GetCurrent());

  // Run button
  left_btn.SetParams("Run", 0, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3, display_drv.GetScreenW() / 2 - BORDER_W, Font_8x12::GetInstance().GetCharH() * 3, true);
  left_btn.SetCallback(AppTask::GetCurrent());
  // Stop button
  right_btn.SetParams("Stop", display_drv.GetScreenW() - left_btn.GetWidth(), left_btn.GetStartY(), left_btn.GetWidth(), left_btn.GetHeight(), true);
  right_btn.SetCallback(AppTask::GetCurrent());

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result OverrideCtrlScr::Show()
{
  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].Show(100);
    dw_real_name[i].Show(100);
  }

  // Feed objects
  feed_dw.Show(100);
  feed_name.Show(100);
  feed_reset_btn.Show(100);

  // Speed objects
  speed_dw.Show(100);
  speed_name.Show(100);
  speed_reset_btn.Show(100);

  // Coolant buttons
  flood_btn.Show(100);
  mist_btn.Show(100);

  // Run button
  left_btn.Show(102);
  // Stop button
  right_btn.Show(102);

  // Set encoder callback handler
  InputDrv::GetInstance().AddEncoderCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessEncoderCallback), this, enc_cble);
  // Set callback handler for left and right buttons
  InputDrv::GetInstance().AddButtonsCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this, InputDrv::BTNM_LEFT | InputDrv::BTNM_RIGHT, btn_cble);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result OverrideCtrlScr::Hide()
{
  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);
  // Delete buttons callback handler
  InputDrv::GetInstance().DeleteButtonsCallbackHandler(btn_cble);

  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].Hide();
    dw_real_name[i].Hide();
  }

  // Feed objects
  feed_dw.Hide();
  feed_name.Hide();
  feed_reset_btn.Hide();

  // Speed objects
  speed_dw.Hide();
  speed_name.Hide();
  speed_reset_btn.Hide();

  // Coolant buttons
  flood_btn.Hide();
  mist_btn.Hide();

  // Go button
  left_btn.Hide();
  // Reset button
  right_btn.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result OverrideCtrlScr::TimerExpired(uint32_t interval)
{
  Result result = Result::RESULT_OK;

  // Update left button text
  if(grbl_comm.GetState() == GrblComm::RUN)
  {
    left_btn.SetString("Hold");
  }
  else
  {
    left_btn.SetString("Run");
  }

  // Update numbers with current position and position difference
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].SetNumber(grbl_comm.GetAxisPosition(i));
  }

  // Update numbers with current overrides
  feed_dw.SetNumber(grbl_comm.GetFeedOverride());
  speed_dw.SetNumber(grbl_comm.GetSpeedOverride());

  // Set coolant state
  flood_btn.SetColor(grbl_comm.GetCoolantFlood() ? COLOR_GREEN : COLOR_WHITE);
  mist_btn.SetColor(grbl_comm.GetCoolantMist() ? COLOR_GREEN : COLOR_WHITE);

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
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result OverrideCtrlScr::ProcessCallback(const void* ptr)
{
  // Process Run button. Since we can call this handler after press of physical
  // button, we have to check if Run button is active.
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
  // Process Reset button
  else if(ptr == &right_btn)
  {
    // Send Stop command
    grbl_comm.Stop();
  }
  else if(ptr == &feed_dw)
  {
    speed_dw.SetSeleced(false);
    feed_dw.SetSeleced(true);
  }
  else if(ptr == &feed_reset_btn)
  {
    grbl_comm.FeedReset();
  }
  else if(ptr == &speed_dw)
  {
    feed_dw.SetSeleced(false);
    speed_dw.SetSeleced(true);
  }
  else if(ptr == &speed_reset_btn)
  {
    grbl_comm.SpeedReset();
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
    ; // Do nothing
  }
  // Always good
  return Result::RESULT_OK;
}

// *************************************************************************
// ***   Private: ProcessEncoderCallback function   ************************
// *************************************************************************
Result OverrideCtrlScr::ProcessEncoderCallback(OverrideCtrlScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    OverrideCtrlScr& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    // Save value to process it later
    if(ths.feed_dw.IsSeleced())
    {
      ths.feed_val += enc_val;
    }

    // Save value to process it later
    if(ths.speed_dw.IsSeleced())
    {
      ths.speed_val += enc_val;
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ProcessButtonCallback function   *****************************
// *****************************************************************************
Result OverrideCtrlScr::ProcessButtonCallback(OverrideCtrlScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    OverrideCtrlScr& ths = *obj_ptr;
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
