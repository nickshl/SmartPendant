//******************************************************************************
//  @file DelayControlScr.cpp
//  @author Nicolai Shlapunov
//
//  @details DelayControlScr: User DelayControlScr Class, implementation
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
#include "DelayControlScr.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
DelayControlScr& DelayControlScr::GetInstance()
{
  static DelayControlScr directcontrolscr;
  return directcontrolscr;
}

// *****************************************************************************
// ***   DelayControlScr Setup   ***********************************************
// *****************************************************************************
Result DelayControlScr::Setup(int32_t y, int32_t height)
{
  int32_t start_y = y + BORDER_W;
  // Data window height
  uint32_t window_height = Font_8x12::GetInstance().GetCharH() * 5u;
  // Fill all windows
  for(uint32_t i = 0u; i < NumberOf(dw); i++)
  {
    // Axis position
    dw[i].SetParams(display_drv.GetScreenW() / 6, start_y + (window_height + BORDER_W*2) * i, (display_drv.GetScreenW() - BORDER_W*2) / 2, window_height, 7u, grbl_comm.GetUnitsPrecision());
    dw[i].SetBorder(BORDER_W, COLOR_RED);
    dw[i].SetDataFont(Font_8x12::GetInstance(), 2u);
    dw[i].SetNumber(grbl_comm.GetAxisPosition(i)); // Get current position at startup
    dw[i].SetUnits(grbl_comm.GetReportUnits(), DataWindow::BOTTOM_RIGHT);
    dw[i].SetCallback(AppTask::GetCurrent());
    dw[i].SetActive(true);
    // Real position
    dw_real[i].SetParams(dw[i].GetEndX() + BORDER_W, dw[i].GetStartY(), display_drv.GetScreenW() - dw[i].GetEndX() - BORDER_W * 2, (window_height - BORDER_W) / 2, 7u, grbl_comm.GetUnitsPrecision());
    dw_real[i].SetBorder(BORDER_W / 2, COLOR_GREY);
    dw_real[i].SetDataFont(Font_8x12::GetInstance());
    dw_real[i].SetNumber(0);
    dw_real[i].SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT, Font_6x8::GetInstance());
    // Position difference
    dw_diff[i].SetParams(dw_real[i].GetStartX(), dw[i].GetEndY() - dw_real[i].GetHeight(), dw_real[i].GetWidth(), dw_real[i].GetHeight(), 7u, grbl_comm.GetUnitsPrecision());
    dw_diff[i].SetBorder(BORDER_W / 2, COLOR_GREY);
    dw_diff[i].SetDataFont(Font_8x12::GetInstance());
    dw_diff[i].SetNumber(0);
    dw_diff[i].SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT, Font_6x8::GetInstance());
    // Axis Name
    axis_names[i].SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
    axis_names[i].SetScale(2u);
    axis_names[i].Move((dw[i].GetStartX() / 2) - (axis_names[i].GetWidth() / 2), (dw[i].GetStartY() + dw[i].GetHeight() / 2) - (axis_names[i].GetHeight() / 2));
  }
  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].SetParams(grbl_comm.IsMetric() ? scale_str_metric[i] : scale_str_imperial[i], i*(display_drv.GetScreenW() / 3) + BORDER_W, dw[NumberOf(dw) - 1u].GetEndY() + BORDER_W*2, display_drv.GetScreenW() / 3 - BORDER_W*2, Font_8x12::GetInstance().GetCharH() * 5, true);
    scale_btn[i].SetCallback(AppTask::GetCurrent());
  }

  // Speed position
  dw_speed.SetParams(display_drv.GetScreenW() / 2, scale_btn[0].GetEndY() + BORDER_W*2, (display_drv.GetScreenW() - BORDER_W*2) / 2,  window_height, 4u, 0u);
  dw_speed.SetBorder(BORDER_W, COLOR_RED);
  dw_speed.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_speed.SetNumber(300);
  dw_speed.SetUnits(grbl_comm.IsMetric() ? "mm/min" : "inches/min", DataWindow::BOTTOM_RIGHT);
  dw_speed.SetCallback(AppTask::GetCurrent());
  dw_speed.SetActive(true);
  // Speed caption
  speed_name.SetParams("SPEED:", 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
  speed_name.SetScale(2u);
  speed_name.Move((display_drv.GetScreenW() / 2 - dw_speed.GetWidth()) / 2, (dw_speed.GetStartY() + dw_speed.GetHeight() / 2) - (speed_name.GetHeight() / 2));

  // Set scale 0.01 mm
  scale = 10;
  // Set corresponded button pressed
  scale_btn[1u].SetPressed(true);

  // X axis mode string
  x_mode_str.SetParams("", dw[GrblComm::AXIS_X].GetStartX() + BORDER_W*2, dw[GrblComm::AXIS_X].GetStartY() + BORDER_W*2, COLOR_WHITE, Font_8x12::GetInstance());

  // Left soft button
  left_btn.SetParams("Go!", 0, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3, display_drv.GetScreenW() / 2 - BORDER_W, Font_8x12::GetInstance().GetCharH() * 3, true);
  left_btn.SetCallback(AppTask::GetCurrent());
  // Right soft button
  right_btn.SetParams("Reset", display_drv.GetScreenW() - left_btn.GetWidth(), left_btn.GetStartY(), left_btn.GetWidth(), left_btn.GetHeight(), true);
  right_btn.SetCallback(AppTask::GetCurrent());

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result DelayControlScr::Show()
{
  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw); i++)
  {
    dw[i].Show(100);
    dw_real[i].Show(100);
    dw_diff[i].Show(100);
    axis_names[i].Show(100);
  }
  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Show(101);
  }

  // Speed window and name
  dw_speed.Show(100);
  speed_name.Show(100);

  // Show X mode string(will be set blank if controllers is not in Lathe mode)
  x_mode_str.Show(100 + 1);

  // Soft Buttons
  left_btn.Show(102);
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
Result DelayControlScr::Hide()
{
  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);
  // Delete buttons callback handler
  InputDrv::GetInstance().DeleteButtonsCallbackHandler(btn_cble);

  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw); i++)
  {
    dw[i].Hide();
    dw_real[i].Hide();
    dw_diff[i].Hide();
    axis_names[i].Hide();
  }
  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Hide();
  }
  // Speed window and name
  dw_speed.Hide();
  speed_name.Hide();

  // Hide X mode string
  x_mode_str.Hide();

  // Soft Buttons
  left_btn.Hide();
  right_btn.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result DelayControlScr::TimerExpired(uint32_t interval)
{
  // In Lathe mode show Radius/Diameter string on top of X window
  if(grbl_comm.GetModeOfOperation() == GrblComm::MODE_OF_OPERATION_LATHE)
  {
    x_mode_str.SetString(grbl_comm.IsLatheDiameterMode() ? "Diameter" : "Radius");
  }
  else
  {
    x_mode_str.SetString("");
  }

  // Get current state
  GrblComm::state_t new_state = GrblComm::GetInstance().GetState();
  // If state has changed
  if(new_state != grbl_state)
  {
    // TODO: any other states combinations?
    // If previous state was JOG state and state is IDLE
    if((grbl_state == GrblComm::JOG) && (new_state == GrblComm::IDLE))
    {
      // Reset numbers with current position. This is needed because in some
      // cases there no ability to match requested position with actual one
      // since we limited by discrete steps. In this case window will stay
      // yellow even axis will not move after hit Go button. To fix that
      // we have reset all data windows after jog is complete.
      for(uint32_t i = 0u; i < NumberOf(dw); i++)
      {
        dw[i].SetNumber(grbl_comm.GetAxisPosition(i));
      }
    }
    // Save current state
    grbl_state = new_state;
  }

  // Update right button text
  if(grbl_comm.GetState() == GrblComm::JOG)
  {
    right_btn.SetString("Stop");
  }
  else
  {
    right_btn.SetString("Reset");
  }

  // Flag to detect change in data
  bool changed = false;

  // Update numbers with current position and position difference
  for(uint32_t i = 0u; i < NumberOf(dw); i++)
  {
    changed |= dw_real[i].SetNumber(grbl_comm.GetAxisPosition(i));
    changed |= dw_diff[i].SetNumber(dw[i].GetNumber() - dw_real[i].GetNumber());
  }

  // If data changed
  if(changed)
  {
    // Update objects on a screen since some windows can change color from
    // yellow to green
    UpdateObjects();
  }

  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result DelayControlScr::ProcessCallback(const void* ptr)
{
  // Process scale buttons
  if(ptr == &scale_btn[0])
  {
    scale = 1;
  }
  else if(ptr == &scale_btn[1])
  {
    scale = 10;
  }
  else if(ptr == &scale_btn[2])
  {
    scale = 100;
  }
  // Process GO button
  else if(ptr == &left_btn)
  {
    grbl_comm.JogMultiple(dw[GrblComm::AXIS_X].GetNumber(), dw[GrblComm::AXIS_Y].GetNumber(), dw[GrblComm::AXIS_Z].GetNumber(), dw_speed.GetNumber() * 100u, true);
  }
  // Process Reset button
  else if(ptr == &right_btn)
  {
    // If we in IDLE state - copy current position
    if(grbl_comm.GetState() == GrblComm::IDLE)
    {
      // Reset numbers with current position
      for(uint32_t i = 0u; i < NumberOf(dw); i++)
      {
        dw[i].SetNumber(grbl_comm.GetAxisPosition(i));
      }
    }
    else // otherwise
    {
      grbl_comm.Stop(); // Send Stop command
    }
  }
  // Process speed data window
  else if(ptr == &dw_speed)
  {
    // Set axis to max
    axis = GrblComm::AXIS_CNT;
  }
  else
  {
    // Check axis data windows
    for(uint32_t i = 0u; i < GrblComm::AXIS_CNT; i++)
    {
      if(ptr == &dw[i])
      {
        // Save axis to control
        axis = (GrblComm::Axis_t)i;
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

// *************************************************************************
// ***   Private: ProcessEncoderCallback function   ************************
// *************************************************************************
Result DelayControlScr::ProcessEncoderCallback(DelayControlScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    DelayControlScr& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    // Process it
    if(enc_val != 0)
    {
      // Change distance
      if(ths.axis < GrblComm::AXIS_CNT)
      {
        ths.dw[ths.axis].SetNumber(ths.dw[ths.axis].GetNumber() + enc_val * ths.scale);
      }
      else // Change speed
      {
        // Calculate new speed
        int32_t new_number = ths.dw_speed.GetNumber() + enc_val;
        // Speed can't be negative(and zero too, but whatever)
        if(new_number < 0) new_number = 0;
        // Set new speed number
        ths.dw_speed.SetNumber(new_number);
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
Result DelayControlScr::ProcessButtonCallback(DelayControlScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    DelayControlScr& ths = *obj_ptr;
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
void DelayControlScr::UpdateObjects(void)
{
  // Update speed window border color
  dw_speed.SetSeleced((axis < GrblComm::AXIS_CNT) ? false : true);

  // Update data windows
  for(uint32_t i = 0u; i < GrblComm::AXIS_CNT; i++)
  {
    // Check if value is different from machine value and this axis will move and set corresponded color
    dw[i].SetBorderColor((dw[i].GetNumber() != grbl_comm.GetAxisPosition(i)) ? COLOR_YELLOW : COLOR_RED);
    // Set selected Data Window flag
    dw[i].SetSeleced(i == axis);
  }

  // Set pressed state for selected scale button and unpressed for unselected ones
  scale_btn[0u].SetPressed(scale == 1);
  scale_btn[1u].SetPressed(scale == 10);
  scale_btn[2u].SetPressed(scale == 100);
}
