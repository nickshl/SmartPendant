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
    dw[i].SetParams(display_drv.GetScreenW() / 6, start_y + (window_height + BORDER_W*2) * i, (display_drv.GetScreenW() - BORDER_W*2) * 4 / 6,  window_height, 4u, 3u);
    dw[i].SetBorder(BORDER_W, COLOR_RED);
    dw[i].SetDataFont(Font_8x12::GetInstance(), 2u);
    dw[i].SetNumber(0);
    dw[i].SetUnits("mm", DataWindow::RIGHT);
    dw[i].SetCallback(AppTask::GetCurrent());
    dw[i].SetActive(true);
    axis_names[i].SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
    axis_names[i].SetScale(2u);
    axis_names[i].Move((dw[i].GetStartX() / 2) - (axis_names[i].GetWidth() / 2), (dw[i].GetStartY() + dw[i].GetHeight() / 2) - (axis_names[i].GetHeight() / 2));
    // Set/Zero button
    set_btn[i].SetParams("<0>", dw[i].GetEndX() + BORDER_W, dw[i].GetStartY(), display_drv.GetScreenW() - dw[i].GetEndX() - BORDER_W * 2, dw[i].GetHeight(), true);
    set_btn[i].SetCallback(AppTask::GetCurrent());
  }
  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].SetParams((i == 0u) ? "0.001 mm" : ((i == 1u) ? "0.01 mm" : "0.1 mm"), i*(display_drv.GetScreenW() / 3) + BORDER_W, dw[NumberOf(dw) - 1u].GetEndY() + BORDER_W*2, display_drv.GetScreenW() / 3 - BORDER_W*2, Font_8x12::GetInstance().GetCharH() * 5, true);
    scale_btn[i].SetCallback(AppTask::GetCurrent());
  }

  // Set scale 0.01 mm
  scale = 10;
  // Set corresponded button pressed
  scale_btn[1u].SetPressed(true);

  // Left soft button
  left_btn.SetParams("Run", 0, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3, display_drv.GetScreenW() / 2 - BORDER_W, Font_8x12::GetInstance().GetCharH() * 3, true);
  left_btn.SetCallback(AppTask::GetCurrent());
  // Right soft button
  right_btn.SetParams("Stop", display_drv.GetScreenW() - left_btn.GetWidth(), left_btn.GetStartY(), left_btn.GetWidth(), left_btn.GetHeight(), true);
  right_btn.SetCallback(AppTask::GetCurrent());

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result DirectControlScr::Show()
{
  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw); i++)
  {
    dw[i].Show(100);
    axis_names[i].Show(100);
    set_btn[i].Show(100);
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
    dw[i].SetSeleced(false);
  }

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
Result DirectControlScr::Hide()
{
  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);
  // Delete buttons callback handler
  InputDrv::GetInstance().DeleteButtonsCallbackHandler(btn_cble);

  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw); i++)
  {
    dw[i].Hide();
    axis_names[i].Hide();
    set_btn[i].Hide();
  }
  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Hide();
  }

  // Soft Buttons
  left_btn.Hide();
  right_btn.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result DirectControlScr::TimerExpired(uint32_t interval)
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

  // Update numbers with current position
  for(uint32_t i = 0u; i < NumberOf(dw); i++)
  {
    dw[i].SetNumber(grbl_comm.GetAxisPosition(i));
  }

  // Update numbers with current position
  for(uint32_t i = 0u; i < GrblComm::AXIS_CNT; i++)
  {
    // If requested position changed
    if(jog_val[i] != 0)
    {
      // Calculate distance - number of encoder clicks mutiplied by click value
      int32_t distance_um = jog_val[i] * scale;
      // Speed in encoder clicks per second
      uint32_t speed = InputDrv::GetInstance().GetEncoderSpeed();
      // 20 clicks per second as minimum speed
      if(speed < 20u) speed = 20u;
      // Speed in um per second
      speed *= scale;
      // Convert speed from um/sec to mm*100/min
      speed = speed * 60u / 10u;

      result = grbl_comm.Jog(i, distance_um, speed);
      // Clear value
      jog_val[i] = 0;
      // One axis at a time
      break;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result DirectControlScr::ProcessCallback(const void* ptr)
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
  else if(ptr == &scale_btn[0])
  {
    // Set pressed state for selected button and unpressed for selected one
    scale_btn[0u].SetPressed(true);
    scale_btn[1u].SetPressed(false);
    scale_btn[2u].SetPressed(false);
    // Save scale to control
    scale = 1;
  }
  else if(ptr == &scale_btn[1])
  {
    // Set pressed state for selected button and unpressed for selected one
    scale_btn[0u].SetPressed(false);
    scale_btn[1u].SetPressed(true);
    scale_btn[2u].SetPressed(false);
    // Save scale to control
    scale = 10;
  }
  else if(ptr == &scale_btn[2])
  {
    // Set pressed state for selected button and unpressed for selected one
    scale_btn[0u].SetPressed(false);
    scale_btn[1u].SetPressed(false);
    scale_btn[2u].SetPressed(true);
    // Save scale to control
    scale = 100;
  }
  else
  {
    // Check axis data windows
    for(uint32_t i = 0u; i < GrblComm::AXIS_CNT; i++)
    {
      if(ptr == &dw[i])
      {
        // Set border to red for all windows
        for(uint32_t j = 0u; j < GrblComm::AXIS_CNT; j++)
        {
          dw[j].SetSeleced(false);
        }
        // Then set border to green for selected one
        dw[i].SetSeleced(true);
        // Save axis to control
        axis = (GrblComm::Axis_t)i;
        // Break the cycle
        break;
      }
      else if(ptr == &set_btn[i])
      {
        grbl_comm.ZeroAxis((GrblComm::Axis_t)i);
        break;
      }
    }
  }
  // Always good
  return Result::RESULT_OK;
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
      ths.jog_val[ths.axis] += enc_val;
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
Result DirectControlScr::ProcessButtonCallback(DirectControlScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    DirectControlScr& ths = *obj_ptr;
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
