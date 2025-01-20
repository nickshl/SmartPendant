//******************************************************************************
//  @file DirectControlScr.cpp
//  @author Nicolai Shlapunov
//
//  @details DirectControlScr: User DirectControlScr Class, implementation
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
#include "DirectControlScr.h"

#include "Application.h"

#include "Version.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
DirectControlScr& DirectControlScr::GetInstance()
{
  static DirectControlScr directcontrolscr;
  return directcontrolscr;
}

// *****************************************************************************
// ***   DirectControlScr Setup   **********************************************
// *****************************************************************************
Result DirectControlScr::Setup(int32_t y, int32_t height)
{
  int32_t start_y = y + BORDER_W;
  // Data window height
  uint32_t window_height = Font_8x12::GetInstance().GetCharH() * 5u;
  // Fill all windows
  for(uint32_t i = 0u; i < NumberOf(dw); i++)
  {
    // Axis position
    dw[i].SetParams(display_drv.GetScreenW() / 6, start_y + (window_height + BORDER_W*2) * i, (display_drv.GetScreenW() - BORDER_W*2) * 4 / 6,  window_height, 7u, grbl_comm.GetUnitsPrecision());
    dw[i].SetBorder(BORDER_W, COLOR_RED);
    dw[i].SetDataFont(Font_8x12::GetInstance(), 2u);
    dw[i].SetNumber(0);
    dw[i].SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
    dw[i].SetCallback(AppTask::GetCurrent());
    dw[i].SetActive(true);
    axis_names[i].SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
    axis_names[i].SetScale(2u);
    axis_names[i].Move((dw[i].GetStartX() / 2) - (axis_names[i].GetWidth() / 2), (dw[i].GetStartY() + dw[i].GetHeight() / 2) - (axis_names[i].GetHeight() / 2));
    // Set/Zero button
    zero_btn[i].SetParams("<0>", dw[i].GetEndX() + BORDER_W, dw[i].GetStartY(), display_drv.GetScreenW() - dw[i].GetEndX() - BORDER_W * 2, dw[i].GetHeight(), true);
    zero_btn[i].SetCallback(AppTask::GetCurrent());
  }
  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    // Calculate scale button width
    uint32_t scale_btn_w = (display_drv.GetScreenW() - BORDER_W * (NumberOf(scale_btn) + 1u)) / NumberOf(scale_btn);
    // Set scale button parameters
    scale_btn[i].SetParams(grbl_comm.IsMetric() ? scale_str_metric[i] : scale_str_imperial[i], BORDER_W + i * (scale_btn_w + BORDER_W), dw[NumberOf(dw) - 1u].GetEndY() + BORDER_W*2, scale_btn_w, window_height, true);
    scale_btn[i].SetCallback(AppTask::GetCurrent());
    scale_btn[i].SetSpacing(3u);
  }

  // Set corresponded button pressed
  scale_btn[2u].SetPressed(true);
  // Set scale 0.01 mm
  scale = grbl_comm.IsMetric() ? scale_val_metric[2u] : scale_val_imperial[2u];

  // X button for Lathe mode to change Radius/Diameter
  x_mode_btn.SetParams("", BORDER_W, dw[GrblComm::AXIS_X].GetStartY(), dw[GrblComm::AXIS_X].GetStartX() - BORDER_W * 2, dw[GrblComm::AXIS_X].GetHeight(), true);
  x_mode_btn.SetCallback(AppTask::GetCurrent());
  // X axis mode string
  x_mode_str.SetParams("", dw[GrblComm::AXIS_X].GetStartX() + BORDER_W*2, dw[GrblComm::AXIS_X].GetStartY() + BORDER_W*2, COLOR_WHITE, Font_8x12::GetInstance());

  // Spindle speed
  spindle_dw.SetParams(BORDER_W, y + height - window_height - BORDER_W, display_drv.GetScreenW() / 2 - BORDER_W * 3 / 2,  window_height, 5u, 0);
  spindle_dw.SetBorder(BORDER_W, COLOR_RED);
  spindle_dw.SetDataFont(Font_8x12::GetInstance(), 2u);
  spindle_dw.SetNumber(grbl_comm.GetSpindleMinSpeed());
  spindle_dw.SetLimits(grbl_comm.GetSpindleMinSpeed(), grbl_comm.GetSpindleMaxSpeed());
  spindle_dw.SetUnits("RPM", DataWindow::RIGHT);
  spindle_dw.SetCallback(AppTask::GetCurrent());
  spindle_dw.SetActive(true);
  spindle_dw.SetSelected(true);
  spindle_name.SetParams("SPINDLE", 0, 0, COLOR_WHITE, Font_8x12::GetInstance());
  spindle_name.Move(spindle_dw.GetStartX() + BORDER_W*2, spindle_dw.GetStartY() + BORDER_W*2);

  // Set scale button parameters
  spindle_dir_btn.SetParams("CW", spindle_dw.GetEndX() + 1 + BORDER_W, spindle_dw.GetStartY(), (spindle_dw.GetWidth() - BORDER_W) / 2, window_height, true);
  spindle_dir_btn.SetCallback(AppTask::GetCurrent());
  spindle_ctrl_btn.SetParams("START", spindle_dir_btn.GetEndX() + 1 + BORDER_W, spindle_dir_btn.GetStartY(), spindle_dir_btn.GetWidth(), window_height, true);
  spindle_ctrl_btn.SetCallback(AppTask::GetCurrent());

  // Version string
  version.SetParams(VERSION, BORDER_W, scale_btn[0].GetEndY() + (spindle_dw.GetStartY() - scale_btn[0].GetEndY() -  Font_8x12::GetInstance().GetCharH()) / 2, COLOR_WHITE, Font_8x12::GetInstance());

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result DirectControlScr::Show()
{
  // Version string
  version.Show(1);

  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw); i++)
  {
    dw[i].Show(100);
    axis_names[i].Show(100);
    zero_btn[i].Show(100);
  }
  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Show(100);
  }
  // Set current axis to none for prevent accidental movement
  axis = GrblComm::AXIS_CNT;
  // Set border to red for all windows
  for(uint32_t i = 0u; i < GrblComm::AXIS_CNT; i++)
  {
    dw[i].SetSelected(false);
  }

  // In Lathe mode show Radius/Diameter string on top of X window and button to change it
  if(grbl_comm.GetModeOfOperation() == GrblComm::MODE_OF_OPERATION_LATHE)
  {
    x_mode_str.Show(100 + 1);
    x_mode_btn.Show(100 - 1);
  }

  // Spindle control
  spindle_dw.Show(100);
  spindle_name.Show(101);
  spindle_dir_btn.Show(100);
  spindle_ctrl_btn.Show(100);

  if(grbl_comm.IsHomingEnabled())
  {
    // Reinit all three Soft Buttons if we have Homing enabled
    Application::GetInstance().InitSoftButtons(true);
    // Show Home button
    middle_btn.SetString("Home");
    middle_btn.Show(102);
  }

  // Soft Buttons
  left_btn.Show(102);
  right_btn.Show(102);

  // Set encoder callback handler
  InputDrv::GetInstance().AddEncoderCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessEncoderCallback), this, enc_cble);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result DirectControlScr::Hide()
{
  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);

  // Version string
  version.Hide();

  // In case if it shown, we should hide it
  change_box.Hide();

  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw); i++)
  {
    dw[i].Hide();
    axis_names[i].Hide();
    zero_btn[i].Hide();
  }
  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Hide();
  }

  // Hide X button
  x_mode_btn.Hide();
  // Hide X mode string
  x_mode_str.Hide();

  // Spindle control
  spindle_dw.Hide();
  spindle_name.Hide();
  spindle_dir_btn.Hide();
  spindle_ctrl_btn.Hide();

  // Soft Buttons
  left_btn.Hide();
  middle_btn.Hide();
  right_btn.Hide();

  // Reinit Soft Buttons to change their size back
  Application::GetInstance().InitSoftButtons(false);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result DirectControlScr::TimerExpired(uint32_t interval)
{
  Result result = Result::RESULT_OK;

  // Update left & right button text
  Application::GetInstance().UpdateLeftButtonText();
  Application::GetInstance().UpdateRightButtonText();

  // In Lathe mode show Radius/Diameter string on top of X window
  if(grbl_comm.GetModeOfOperation() == GrblComm::MODE_OF_OPERATION_LATHE)
  {
    x_mode_str.SetString(grbl_comm.IsLatheDiameterMode() ? "Diameter" : "Radius");
  }

  // Update numbers with current position
  for(uint32_t i = 0u; i < NumberOf(dw); i++)
  {
    dw[i].SetNumber(grbl_comm.GetAxisPosition(i));
  }

  // Update numbers with current position
  for(uint32_t i = 0u; i < GrblComm::AXIS_CNT; i++)
  {
    // If requested position changed
    if(axis_jog_val[i] != 0)
    {
      // Calculate distance - number of encoder clicks multiplied by click value
      int32_t distance = axis_jog_val[i] * scale;
      // Feed in encoder clicks per second
      uint32_t feed = InputDrv::GetInstance().GetEncoderSpeed();
      // 20 clicks per second as minimum feed
      if(feed < 20u) feed = 20u;
      // Feed in units(1 um or 0.0001 inch depend on controller settings) per second
      feed *= scale;
      // Convert feed from units/sec to units*100/min
      feed = feed * 60u / 10u;

      // In Lathe mode we need some changes
      if((i == GrblComm::AXIS_X) && (grbl_comm.GetModeOfOperation() == GrblComm::MODE_OF_OPERATION_LATHE))
      {
        // Invert X axis since clockwise rotation moves cutter to work piece(X decreased)
        // and counterclockwise rotation moves cutter away from work piece(X increased)
        distance = -distance;
      }

      result = grbl_comm.Jog(i, distance, feed, false);

      // Clear value
      axis_jog_val[i] = 0;
      // One axis at a time
      break;
    }
  }

  // Update spindle direction button text if spindle is running
  if(grbl_comm.IsSpindleCCW())
  {
    spindle_dir_btn.SetString("CCW");
  }
  else
  {
    spindle_dir_btn.SetString("CW");
  }

  // Update spindle control button text
  if(grbl_comm.IsSpindleRunning())
  {
    // If spindle is running - control button is stop button
    spindle_ctrl_btn.SetString("STOP");
    // Update current speed
    spindle_dw.SetNumber(grbl_comm.GetSpindleSpeed());
  }
  else
  {
    // If spindle is not running - control button is start button
    spindle_ctrl_btn.SetString("START");
  }

  // Spindle speed
  if(jog_val != 0)
  {
    // Update speed in data window
    spindle_dw.SetNumber(spindle_dw.GetNumber() + jog_val * 10);
    // clear jog value
    jog_val = 0;
    // Update spindle speed if it is running
    if(grbl_comm.IsSpindleRunning())
    {
      grbl_comm.SetSpindleSpeed(spindle_dw.GetNumber(), grbl_comm.IsSpindleCCW());
    }
  }

  // If SmartPendant is in control and state is IDLE or JOG - enable buttons
  if(grbl_comm.IsInControl() && ((grbl_comm.GetState() == GrblComm::IDLE) || (grbl_comm.GetState() == GrblComm::JOG)))
  {
    x_mode_btn.Enable();
    // Enable zero buttons for all axis
    for(uint32_t i = 0u; i < NumberOf(zero_btn); i++)
    {
      zero_btn[i].Enable();
    }
    // Enable spindle control button
    spindle_ctrl_btn.Enable();
    // Spindle dir button should be disabled if spindle is running
    if(grbl_comm.IsSpindleRunning())
    {
      spindle_dir_btn.Disable();
    }
    else
    {
      spindle_dir_btn.Enable();
    }
  }
  else
  {
    change_box.Hide();
    x_mode_btn.Disable();
    // Enable zero buttons for all axis
    for(uint32_t i = 0u; i < NumberOf(zero_btn); i++)
    {
      zero_btn[i].Disable();
    }
    // Disable spindle buttons
    spindle_ctrl_btn.Disable();
    spindle_dir_btn.Disable();
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   UnpressButtons function   *********************************************
// *****************************************************************************
void DirectControlScr::UnpressButtons(void)
{
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].SetPressed(false);
  }
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result DirectControlScr::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  // Process Left & Right buttons
  if((ptr == &left_btn) || (ptr == &right_btn))
  {
    result = Result::ERR_UNHANDLED_REQUEST; // For Application to handle it
  }
  else if(ptr == &middle_btn)
  {
    grbl_comm.Homing();
  }
  // Process X button
  else if(ptr == &x_mode_btn)
  {
    // Invert mode
    grbl_comm.IsLatheDiameterMode() ? grbl_comm.SetLatheRadiusMode() : grbl_comm.SetLatheDiameterMode();
  }
  // Process change box callback
  else if(ptr == &change_box)
  {
    grbl_comm.SetAxisPosition(change_box.GetId(), change_box.GetValue());
  }
  // Process spindle control button
  else if(ptr == &spindle_ctrl_btn)
  {
    if(grbl_comm.IsSpindleRunning())
    {
      grbl_comm.StopSpindle();
    }
    else
    {
      grbl_comm.SetSpindleSpeed(spindle_dw.GetNumber(), grbl_comm.IsSpindleCCW());
    }
  }
  // Process spindle direction button
  else if(ptr == &spindle_dir_btn)
  {
    grbl_comm.SetSpindleDirection(!grbl_comm.IsSpindleCCW());
  }
  else
  {
    uint32_t i = 0u;
    // Try to find button
    for(; i < NumberOf(scale_btn); i++)
    {
      if(ptr == &scale_btn[i])
      {
        // Set unpressed state for all buttons
        UnpressButtons();
        // Set pressed state for selected one
        scale_btn[i].SetPressed(true);
        // Save scale to control
        scale = grbl_comm.IsMetric() ? scale_val_metric[i] : scale_val_imperial[i];
        break;
      }
    }
    // If previous cycle isn't found button
    if(i == NumberOf(scale_btn))
    {
      // Check axis data windows
      for(uint32_t i = 0u; i < GrblComm::AXIS_CNT; i++)
      {
        // If we tapped on already selected button
        if((ptr == &dw[i]) && dw[i].IsSelected())
        {
          // Setup object to change numerical parameters
          change_box.Setup(grbl_comm.GetAxisName(i), grbl_comm.GetReportUnits(), dw[i].GetNumber(), -10000000, 10000000, grbl_comm.GetUnitsPrecision());
          // Set AppTask
          change_box.SetCallback(AppTask::GetCurrent());
          // Save axis index as ID
          change_box.SetId(i);
          // Show change box
          change_box.Show(10000u);
        }
        else if(ptr == &dw[i])
        {
          // Set border to red for all windows
          for(uint32_t j = 0u; j < GrblComm::AXIS_CNT; j++)
          {
            dw[j].SetSelected(false);
          }
          // Unselect spindle data window
          spindle_dw.SetSelected(false);
          // Then set border to green for selected one
          dw[i].SetSelected(true);
          // Save axis to control
          axis = (GrblComm::Axis_t)i;
          // Break the cycle
          break;
        }
        else if(ptr == &zero_btn[i])
        {
          grbl_comm.ZeroAxis((GrblComm::Axis_t)i);
        }
      }
      if(ptr == &spindle_dw)
      {
        // Select spindle data window
        spindle_dw.SetSelected(true);
        // Not an axis
        axis = GrblComm::AXIS_CNT;
        // Set border to red for all windows
        for(uint32_t j = 0u; j < GrblComm::AXIS_CNT; j++)
        {
          dw[j].SetSelected(false);
        }
      }
    }
  }

  // Return result
  return result;
}

// *************************************************************************
// ***   Private: ProcessEncoderCallback function   ************************
// *************************************************************************
Result DirectControlScr::ProcessEncoderCallback(DirectControlScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    DirectControlScr& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    // Save jogging value to send it later
    if(ths.axis < GrblComm::AXIS_CNT)
    {
      ths.axis_jog_val[ths.axis] += enc_val;
    }
    else
    {
      ths.jog_val += enc_val;
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
DirectControlScr::DirectControlScr() : left_btn(Application::GetInstance().GetLeftButton()),
                                       middle_btn(Application::GetInstance().GetMiddleButton()),
                                       right_btn(Application::GetInstance().GetRightButton()),
                                       change_box(Application::GetInstance().GetChangeValueBox()){};
