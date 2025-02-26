//******************************************************************************
//  @file RotaryTableScr.cpp
//  @author Nicolai Shlapunov
//
//  @details RotaryTableScr: User RotaryTableScr Class, implementation
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
#include "RotaryTableScr.h"

#include "Application.h"

#include <cmath>

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
RotaryTableScr& RotaryTableScr::GetInstance()
{
  static RotaryTableScr rotarytablescr;
  return rotarytablescr;
}

// *****************************************************************************
// ***   RotaryTableScr Setup   ************************************************
// *****************************************************************************
Result RotaryTableScr::Setup(int32_t y, int32_t height)
{
  int32_t start_y = y + BORDER_W;

  // *** TODO: REMOVE ! ********************************************************
//  dcx = display_drv.GetScreenW() / 2;
//  dcy = display_drv.GetScreenH() / 2;
//  dpx = display_drv.GetScreenW() / 3;
//  dpy = display_drv.GetScreenH() / 2 - display_drv.GetScreenW() / 3;
//  cir.SetParams(dpx, dpy, 10, COLOR_WHITE, true);
  // *** TODO: REMOVE ! ********************************************************

  center.SetParams("CENTER", 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
  center.SetScale(2u);
  center.Move((display_drv.GetScreenW() / 2) - (center.GetWidth() / 2), start_y);
  // Data window height
  uint32_t window_height = Font_8x12::GetInstance().GetCharH() * 5u;
  // Fill all windows
  for(uint32_t i = 0u; i < NumberOf(center_dw); i++)
  {
    // Axis position
    center_dw[i].SetParams(BORDER_W + (i ? center_dw[i - 1u].GetEndX() + 1 : 0), center.GetEndY() + BORDER_W, (display_drv.GetScreenW() - BORDER_W * (1 + NumberOf(center_dw))) / NumberOf(center_dw),  window_height, 7u, grbl_comm.GetUnitsPrecision());
    center_dw[i].SetBorder(BORDER_W, COLOR_RED);
    center_dw[i].SetDataFont(Font_8x12::GetInstance(), 2u);
    center_dw[i].SetNumber(grbl_comm.GetAxisPosition(i)); // Get current position at startup
    center_dw[i].SetUnits(grbl_comm.GetReportUnits(), DataWindow::BOTTOM_RIGHT);
    center_dw[i].SetCallback(AppTask::GetCurrent());
    center_dw[i].SetActive(true);
    // Axis Name
    center_axis_name[i].SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
    center_axis_name[i].Move(center_dw[i].GetStartX() + BORDER_W*2, center_dw[i].GetStartY() + BORDER_W*2);
    //center_axis_name[i].Move((center_dw[i].GetStartX() / 2) - (center_axis_name[i].GetWidth() / 2), (center_dw[i].GetStartY() + center_dw[i].GetHeight() / 2) - (center_axis_name[i].GetHeight() / 2));
  }

  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].SetParams(grbl_comm.IsMetric() ? scale_str_metric[i] : scale_str_imperial[i], i*(display_drv.GetScreenW() / 3) + BORDER_W, center_dw[NumberOf(center_dw) - 1u].GetEndY() + BORDER_W*2, display_drv.GetScreenW() / 3 - BORDER_W*2, Font_8x12::GetInstance().GetCharH() * 5, true);
    scale_btn[i].SetCallback(AppTask::GetCurrent());
    scale_btn[i].SetSpacing(3u);
  }
  // Set scale to the middle
  scale = grbl_comm.IsMetric() ? scale_val_metric[1u] : scale_val_imperial[1u];
  // Set corresponded button pressed
  scale_btn[1u].SetPressed(true);

  // Radius position
  radius_dw.SetParams(BORDER_W, scale_btn[0].GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2,  window_height, 15u, grbl_comm.GetUnitsPrecision());
  radius_dw.SetBorder(BORDER_W, COLOR_RED);
  radius_dw.SetDataFont(Font_8x12::GetInstance(), 2u);
  radius_dw.SetLimits(1, INT32_MAX);
  radius_dw.SetNumber(1);
  radius_dw.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  radius_dw.SetCallback(AppTask::GetCurrent());
  radius_dw.SetActive(true);
  // Radius caption
  radius_name.SetParams("RADIUS", radius_dw.GetStartX() + BORDER_W*2, radius_dw.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // Z axis position
  z_axis_dw.SetParams(BORDER_W, radius_dw.GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2,  window_height, 15u, grbl_comm.GetUnitsPrecision());
  z_axis_dw.SetBorder(BORDER_W, COLOR_RED);
  z_axis_dw.SetDataFont(Font_8x12::GetInstance(), 2u);
  z_axis_dw.SetNumber(0);
  z_axis_dw.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  z_axis_dw.SetCallback(AppTask::GetCurrent());
  z_axis_dw.SetActive(true);
  // Z axis caption
  z_axis_name.SetParams(grbl_comm.GetAxisName(GrblComm::AXIS_Z), z_axis_dw.GetStartX() + BORDER_W*2, z_axis_dw.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // Arc length position
  arc_dw.SetParams(BORDER_W, z_axis_dw.GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2,  window_height, 15u, grbl_comm.GetUnitsPrecision());
  arc_dw.SetBorder(BORDER_W, COLOR_RED);
  arc_dw.SetDataFont(Font_8x12::GetInstance(), 2u);
  arc_dw.SetNumber(0);
  arc_dw.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  arc_dw.SetCallback(AppTask::GetCurrent());
  arc_dw.SetActive(true);
  // Arc caption
  arc_name.SetParams("ARC LENGTH", arc_dw.GetStartX() + BORDER_W*2, arc_dw.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result RotaryTableScr::Show()
{
  // *** TODO: REMOVE ! ********************************************************
//  cir.SetParams(start_x, start_y, 10, COLOR_WHITE, true);
//  cir.Show(1000);
  // *** TODO: REMOVE ! ********************************************************

  // Save current coordinates as base for arc
  start_x = grbl_comm.GetAxisPosition(GrblComm::AXIS_X);
  start_y = grbl_comm.GetAxisPosition(GrblComm::AXIS_Y);
  // Save current coordinates
  current_x = start_x;
  current_y = start_y;
  // Recalculate radius
  radius = (uint32_t)(sqrt(pow(start_x - center_x, 2) + pow(start_y - center_y, 2)));
  // Set radius - two numbers should match
  radius_dw.SetNumber(radius);
  // Clear length
  arc_length = 0;
  // Set arc length
  arc_dw.SetNumber(arc_length);

  // Show "Center" string
  center.Show(100);
  // Axis data
  for(uint32_t i = 0u; i < NumberOf(center_dw); i++)
  {
    center_dw[i].Show(100);
    center_axis_name[i].Show(101);
  }
  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Show(101);
  }
  // Radius window and name
  radius_dw.Show(100);
  radius_name.Show(100);
  // Z axis window and name
  z_axis_dw.Show(100);
  z_axis_name.Show(100);
  // Arc length window and name
  arc_dw.Show(100);
  arc_name.Show(100);

  // Real X & Y axis data
  for(uint32_t i = 0u; i < 2u; i++)
  {
    DataWindow& dw_real = Application::GetInstance().GetRealDataWindow(i);
    String& dw_real_name = Application::GetInstance().GetRealDataWindowNameString(i);

    dw_real.SetParams(BORDER_W / 2 + (display_drv.GetScreenW() / 2) * i, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3 + BORDER_W / 2, (display_drv.GetScreenW() - BORDER_W*2) / 2, Font_8x12::GetInstance().GetCharH() * 3 - BORDER_W, 14u, grbl_comm.GetUnitsPrecision());
    dw_real.SetBorder(BORDER_W / 2, COLOR_GREY);
    dw_real.SetDataFont(Font_8x12::GetInstance());
    dw_real.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT, Font_6x8::GetInstance());
    // Axis Name
    dw_real_name.SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_10x18::GetInstance());
    dw_real_name.Move(dw_real.GetStartX() + BORDER_W, dw_real.GetStartY() + (dw_real.GetHeight() - dw_real_name.GetHeight()) / 2);

    dw_real.Show(100);
    dw_real_name.Show(100);
  }

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
Result RotaryTableScr::Hide()
{
  // *** TODO: REMOVE ! ********************************************************
//  cir.Hide();
  // *** TODO: REMOVE ! ********************************************************

  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);
  // Delete buttons callback handler
  InputDrv::GetInstance().DeleteButtonsCallbackHandler(btn_cble);

  // Hide "Center" string
  center.Hide();
  // Axis data
  for(uint32_t i = 0u; i < NumberOf(center_dw); i++)
  {
    center_dw[i].Hide();
    center_axis_name[i].Hide();
  }
  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Hide();
  }
  // Radius window and name
  radius_dw.Hide();
  radius_name.Hide();
  // Z axis window and name
  z_axis_dw.Hide();
  z_axis_name.Hide();
  // Arc length window and name
  arc_dw.Hide();
  arc_name.Hide();

  // Real X & Y axis data
  for(uint32_t i = 0u; i < GrblComm::AXIS_CNT; i++)
  {
    Application::GetInstance().GetRealDataWindow(i).Hide();
    Application::GetInstance().GetRealDataWindowNameString(i).Hide();
  }

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result RotaryTableScr::TimerExpired(uint32_t interval)
{
  Result result = Result::RESULT_OK;

  // Feed in encoder clicks per second
  uint32_t feed = InputDrv::GetInstance().GetEncoderSpeed();
  // 20 clicks per second as minimum speed
  if(feed < 20u) feed = 20u;
  // Feed in um per second
  feed *= scale;
  // Convert feed from um/sec to mm*100/min
  feed = feed * 60u / 10u;

  // Update numbers with current position
  z_axis_dw.SetNumber(grbl_comm.GetAxisPosition(GrblComm::AXIS_Z));

  // *** Process X or Y center change ******************************************
  if((center_x != center_dw[GrblComm::AXIS_X].GetNumber()) || (center_y != center_dw[GrblComm::AXIS_Y].GetNumber()))
  {
    // Set new center coordinate
    center_dw[GrblComm::AXIS_X].SetNumber(center_x);
    // Set new center coordinate
    center_dw[GrblComm::AXIS_Y].SetNumber(center_y);
    // Recalculate radius
    radius = (uint32_t)(sqrt(pow(start_x - center_x, 2) + pow(start_y - center_y, 2)));
    // Set new radius number
    radius_dw.SetNumber(radius);
    // Clear length
    arc_length = 0;
    // Set arc length
    arc_dw.SetNumber(arc_length);
  }

  // *** Process Z axis position change ****************************************
  if(z_val != 0)
  {
    // Execute Jog
    result = grbl_comm.Jog(GrblComm::AXIS_Z, z_val, feed, false);
    // Clear value
    z_val = 0;
  }

  // *** Process radius change *************************************************
  if(radius_change != 0)
  {
    // Recalculate new current points using change in radius
    FindNewPointByRadius(current_x, current_y, center_x, center_y, radius_change);
    // Recalculate radius itself
    radius = (uint32_t)(sqrt(pow(current_x - center_x, 2) + pow(current_y - center_y, 2)));
    // Update radius window with new value
    radius_dw.SetNumber(radius);
    // Clear radius change
    radius_change = 0;
    // Update start points
    start_x = current_x;
    start_y = current_y;
    // Clear arc length
    arc_length = 0;
    // Set arc length
    arc_dw.SetNumber(arc_length);
    // Move tool for new radius
    result = grbl_comm.JogMultiple(start_x, start_y, grbl_comm.GetAxisPosition(GrblComm::AXIS_Z), feed, true);
  }

  // *** Process arc length change *********************************************
  if(arc_length != arc_dw.GetNumber())
  {
    // Find difference between old and new length
    int32_t diff = arc_dw.GetNumber() - arc_length;
    // Update length window with new value
    arc_dw.SetNumber(arc_length);

    // Set current points to arc start for calculation
    double new_x = start_x;
    double new_y = start_y;
    // Calculate new position
    FindArcSecondPoint(new_x, new_y, center_x, center_y, abs(arc_length), arc_length >= 0);

    // Since we have finite resolution in 1 um, we have to check if new point is the same as old point after round
    if(((int32_t)new_x != (int32_t)current_x) || ((int32_t)new_y != (int32_t)current_y))
    {
      // Save new point
      current_x = new_x;
      current_y = new_y;
      // Run Jog command
      result = grbl_comm.JogArcXYR((int32_t)current_x, (int32_t)current_y, radius, feed, diff < 0, true);
    }

    // *** TODO: REMOVE ! ******************************************************
    {
//      double tpx = dpx;
//      double tpy = dpy;
//      FindArcSecondPoint(tpx, tpy, dcx, dcy, abs(dl), dl >= 0);
//      cir.Move(tpx, tpy);
    }
    // *** TODO: REMOVE ! ******************************************************
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result RotaryTableScr::ProcessCallback(const void* ptr)
{
  // Process radius data window
  if(ptr == &radius_dw)
  {
    // Set focus to radius data window
    focus = FOCUS_RADIUS;
  }
  // Process Z axis data window
  else if(ptr == &z_axis_dw)
  {
    // Set focus to Z axis data window
    focus = FOCUS_Z_AXIS;
  }
  // Process arc length data window
  else if(ptr == &arc_dw)
  {    // Set focus to Z axis data window
    focus = FOCUS_ARC_LENGTH;
  }
  else
  {
    // Process scale buttons
    for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
    {
      if(ptr == &scale_btn[i])
      {
        scale = grbl_comm.IsMetric() ? scale_val_metric[i] : scale_val_imperial[i];
      }
    }

    // Check axis data windows
    for(uint32_t i = 0u; i < NumberOf(center_dw); i++)
    {
      if(ptr == &center_dw[i])
      {
        // Save axis to control
        focus = (Focus_t)(FOCUS_CENTER_X_AXIS + i);
        // Break the cycle
        break;
      }
    }
  }

  // Update objects on a screen
  UpdateObjects();

  // Always good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Private: ProcessEncoderCallback function   ****************************
// *****************************************************************************
Result RotaryTableScr::ProcessEncoderCallback(RotaryTableScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    RotaryTableScr& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    // Process it
    if(enc_val != 0)
    {
//      // TODO: Remove!
//      ths.dl += enc_val;

      // Change center coordinates
      if(ths.focus == FOCUS_CENTER_X_AXIS)
      {
        // Update center X coordinate
        ths.center_x += enc_val * ths.scale;
      }
      else if(ths.focus == FOCUS_CENTER_Y_AXIS)
      {
        // Update center Y coordinate
        ths.center_y += enc_val * ths.scale;
      }
      else if(ths.focus == FOCUS_RADIUS)
      {
        // Update center Y coordinate
        ths.radius_change += enc_val * ths.scale;
      }
      else if(ths.focus == FOCUS_Z_AXIS)
      {
        // Set new z number
        ths.z_val += enc_val * ths.scale;
      }
      else if(ths.focus == FOCUS_ARC_LENGTH)
      {
        // Set new arc length number
        ths.arc_length += enc_val * ths.scale;
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
// ***   Private: ProcessButtonCallback function   *****************************
// *****************************************************************************
Result RotaryTableScr::ProcessButtonCallback(RotaryTableScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    RotaryTableScr& ths = *obj_ptr;
    // Get pressed button
    InputDrv::ButtonCallbackData btn = *((InputDrv::ButtonCallbackData*)ptr);

    // Right button - Reset values or send Stop command to GRBL if in movement
    if(btn.btn == InputDrv::BTN_RIGHT)
    {
      // If GRBL in IDLE, SLEEP or UNKNOWN state
      if( (GrblComm::GetInstance().GetState() == GrblComm::IDLE) ||
          (GrblComm::GetInstance().GetState() == GrblComm::SLEEP) ||
          (GrblComm::GetInstance().GetState() == GrblComm::UNKNOWN) )
      {
        // If button pressed
        if(btn.state == true)
        {
          // Reset numbers with current position
          for(uint32_t i = 0u; i < NumberOf(center_dw); i++)
          {
            ths.center_dw[i].SetNumber(ths.grbl_comm.GetAxisPosition(i));
          }
        }
      }
      else
      {
        // If button pressed
        if(btn.state == true)
        {
          // TODO: send reset to GRBL
        }
      }
    }
    else
    {
      ; // Do nothing - MISRA rule
    }

    // Update objects on a screen
    ths.UpdateObjects();

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: Update function   ********************************************
// *****************************************************************************
void RotaryTableScr::UpdateObjects(void)
{
  // Update windows border color
  radius_dw.SetSelected((focus  == FOCUS_RADIUS) ? true : false);
  z_axis_dw.SetSelected((focus == FOCUS_Z_AXIS) ? true : false);
  arc_dw.SetSelected((focus == FOCUS_ARC_LENGTH) ? true : false);

  // Update position window border colors
  for(uint32_t i = 0u; i <= FOCUS_CENTER_Y_AXIS; i++)
  {
    center_dw[i].SetSelected((i == focus) ? true : false);
  }

  // Set pressed state for selected scale button and unpressed for unselected ones
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].SetPressed(scale == (grbl_comm.IsMetric() ? scale_val_metric[i] : scale_val_imperial[i]));
  }
}

// *****************************************************************************
// ***   Private: FindArcSecondPoint function   ********************************
// *****************************************************************************
void RotaryTableScr::FindArcSecondPoint(double& xp, double& yp, double xc, double yc, double l, bool clockwise)
{
  // Find radius for given points
  double r = sqrt(pow(xp - xc, 2) + pow(yp - yc, 2));
  // Find angle
  double angle = atan2(yp - yc, xp - xc);
  // Check direction
  if(clockwise == true)
  {
    angle = angle - l / r;
  }
  else
  {
    angle = angle + l / r;
  }
  // Calculate result
  xp = xc + r * cos(angle);
  yp = yc + r * sin(angle);
}

// *****************************************************************************
// ***   Private: FindNewPointByRadius function   ******************************
// *****************************************************************************
void RotaryTableScr::FindNewPointByRadius(double& xp, double& yp, double xc, double yc, double r_change)
{
  // Find radius for given points
  double r = sqrt(pow(xp - xc, 2) + pow(yp - yc, 2));
  // Find angle
  double angle = atan2(yp - yc, xp - xc);
  // Calculate result
  xp = (int32_t)(xc + (r + r_change) * cos(angle));
  yp = (int32_t)(yc + (r + r_change) * sin(angle));
}
