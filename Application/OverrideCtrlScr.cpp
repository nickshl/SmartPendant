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

#include "Application.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
OverrideCtrlScr& OverrideCtrlScr::GetInstance()
{
  static OverrideCtrlScr overridectrlscr;
  return overridectrlscr;
}

// *****************************************************************************
// ***   OverrideCtrlScr Setup   ***********************************************
// *****************************************************************************
Result OverrideCtrlScr::Setup(int32_t y, int32_t height)
{
  // Data window height
  uint32_t window_height = Font_8x12::GetInstance().GetCharH() * 5u;

  // Start position for Feed Data Window: border, names, data window, two borders
  int32_t start_y = y + BORDER_W + Font_10x18::GetInstance().GetCharH() + BORDER_W + (Font_10x18::GetInstance().GetCharH() + Font_6x8::GetInstance().GetCharH()*2 + BORDER_W);
  // If there more than 3 axis, we need second row
//  if(grbl_comm.GetNumberOfAxis() > 3) start_y *= 2;
  // Add gap between data windows and Feed & Speed
  start_y += BORDER_W*2;

  // Feed override
  feed_dw.SetParams(display_drv.GetScreenW() / 4, start_y, (display_drv.GetScreenW() - display_drv.GetScreenW() / 4 - BORDER_W) / 2,  window_height, 3u, 0u);
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

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result OverrideCtrlScr::Show()
{
  // Fill all windows
  for(uint32_t i = 0u; i < grbl_comm.GetLimitedNumberOfAxis(3u); i++)
  {
    DataWindow& dw_real = Application::GetInstance().GetRealDataWindow(i);
    String& dw_real_name = Application::GetInstance().GetRealDataWindowNameString(i);

    // Real position
    dw_real.SetParams(BORDER_W + ((display_drv.GetScreenW() - BORDER_W * 4) / 3 + BORDER_W) * i, Application::GetInstance().GetScreenStartY() + BORDER_W*2 + Font_10x18::GetInstance().GetCharH(), (display_drv.GetScreenW() - BORDER_W * 4) / 3, Font_10x18::GetInstance().GetCharH() + Font_6x8::GetInstance().GetCharH()*2 + BORDER_W, 8u, grbl_comm.GetUnitsPrecision());
    dw_real.SetBorder(BORDER_W / 2, COLOR_GREY);
    dw_real.SetDataFont(Font_10x18::GetInstance());
    dw_real.SetUnits(grbl_comm.GetReportUnits(), DataWindow::BOTTOM_RIGHT, Font_6x8::GetInstance());
    // Axis Name
    dw_real_name.SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_10x18::GetInstance());
    dw_real_name.Move(dw_real.GetStartX() + (dw_real.GetWidth() - dw_real_name.GetWidth()) / 2, dw_real.GetStartY() - BORDER_W - dw_real_name.GetHeight());

    dw_real.Show(100);
    dw_real_name.Show(100);
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

  // Axis data
  for(uint32_t i = 0u; i < GrblComm::AXIS_CNT; i++)
  {
    Application::GetInstance().GetRealDataWindow(i).Hide();
    Application::GetInstance().GetRealDataWindowNameString(i).Hide();
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

  // Update left & right button text
  Application::GetInstance().UpdateLeftButtonText();
  Application::GetInstance().UpdateRightButtonText();

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
  Result result = Result::RESULT_OK;

  if(ptr == &feed_dw)
  {
    speed_dw.SetSelected(false);
    feed_dw.SetSelected(true);
  }
  else if(ptr == &feed_reset_btn)
  {
    grbl_comm.FeedReset();
  }
  else if(ptr == &speed_dw)
  {
    feed_dw.SetSelected(false);
    speed_dw.SetSelected(true);
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
    result = Result::ERR_UNHANDLED_REQUEST; // For Application to handle it
  }

  // Return result
  return result;
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
    if(ths.feed_dw.IsSelected())
    {
      ths.feed_val += enc_val;
    }

    // Save value to process it later
    if(ths.speed_dw.IsSelected())
    {
      ths.speed_val += enc_val;
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *************************************************************************
// ***   Private constructor   *********************************************
// *************************************************************************
OverrideCtrlScr::OverrideCtrlScr() : left_btn(Application::GetInstance().GetLeftButton()),
                                     right_btn(Application::GetInstance().GetRightButton()) {};
