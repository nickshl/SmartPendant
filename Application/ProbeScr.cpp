//******************************************************************************
//  @file ProbeScr.cpp
//  @author Nicolai Shlapunov
//
//  @details ProbeScr: User ProbeScr Class, implementation
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
#include "ProbeScr.h"

#include "Application.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
ProbeScr& ProbeScr::GetInstance()
{
  static ProbeScr probescr;
  return probescr;
}

// *****************************************************************************
// ***   ProbeScr Setup   ******************************************************
// *****************************************************************************
Result ProbeScr::Setup(int32_t y, int32_t height)
{
  // Tabs for screens(first call to set parameters)
  tabs.SetParams(0, y, display_drv.GetScreenW(), 40, Tabs::MAX_TABS);
  // Screens & Captions
  tabs_cnt = 0u; // Clear tabs since Setup() can be called multiple times
  // Fill tabs
  tabs.SetText(tabs_cnt, "Center", "Finder", Font_10x18::GetInstance());
  tab[tabs_cnt++] = &CenterFinderTab::GetInstance();
  tabs.SetText(tabs_cnt, "Edge", "Finder", Font_10x18::GetInstance());
  tab[tabs_cnt++] = &EdgeFinderTab::GetInstance();
  tabs.SetText(tabs_cnt, "Tool", "Offset", Font_10x18::GetInstance());
  tab[tabs_cnt++] = &ToolOffsetTab::GetInstance();
  // Tabs for screens(second call to resize to actual numbers of pages)
  tabs.SetParams(0, y, display_drv.GetScreenW(), 40, tabs_cnt);
  // Set callback
  tabs.SetCallback(AppTask::GetCurrent());

  // Setup all screens
  for(uint32_t i = 0u; i < tabs_cnt; i++)
  {
    tab[i]->Setup(y + 40, height - 40);
  }

  // Set index
  tab_idx = 0u;

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result ProbeScr::Show()
{
  // Show tabs
  tabs.Show(2000);

  // Soft Buttons
  left_btn.Show(102);
  right_btn.Show(102);

  // Set callback handler for left and right buttons
  InputDrv::GetInstance().AddButtonsCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this, InputDrv::BTNM_LEFT_DOWN | InputDrv::BTNM_RIGHT_DOWN, btn_cble);

  // Show screen
  tab[tab_idx]->Show();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result ProbeScr::Hide()
{
  // Delete buttons callback handler
  InputDrv::GetInstance().DeleteButtonsCallbackHandler(btn_cble);

  // Show tabs
  tabs.Hide();

  // Soft Buttons
  left_btn.Hide();
  right_btn.Hide();

  // Hide screen
  tab[tab_idx]->Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result ProbeScr::TimerExpired(uint32_t interval)
{
  Result result = Result::RESULT_OK;

  // Update right button text
  Application::GetInstance().UpdateRightButtonText();

  // Process timer
  tab[tab_idx]->TimerExpired(interval);

  // Return ok - we don't check semaphore give error, because we don't need to.
  return result;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result ProbeScr::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  if(ptr == &right_btn)
  {
    result = Result::ERR_UNHANDLED_REQUEST; // For Application to handle it
  }
  else if(ptr == &tabs) // Change tab when another tab selected
  {
    ChangeTab(tabs.GetSelectedTab());
  }
  else
  {
    result = tab[tab_idx]->ProcessCallback(ptr);
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: EnableScreenChange function   *********************************
// *****************************************************************************
void ProbeScr::EnableScreenChange()
{
  tabs.Enable();
  Application::GetInstance().EnableScreenChange();
}

// *****************************************************************************
// ***   Public: DisableScreenChange function   ********************************
// *****************************************************************************
void ProbeScr::DisableScreenChange()
{
  tabs.Disable();
  Application::GetInstance().DisableScreenChange();
}

// *****************************************************************************
// ***   Private: ProcessButtonCallback function   *****************************
// *****************************************************************************
Result ProbeScr::ProcessButtonCallback(ProbeScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ProbeScr& ths = *obj_ptr;
    // Get pressed button
    InputDrv::ButtonCallbackData btn = *((InputDrv::ButtonCallbackData*)ptr);

    // Change tab on button release only
    if((ths.tabs.IsEnabled()) && (btn.state == false))
    {
      // Buttons to switch tabs
      if(btn.btn == InputDrv::BTN_LEFT_DOWN)
      {
        if(ths.tab_idx > 0u) ths.ChangeTab(ths.tab_idx - 1u);
      }
      else if(btn.btn == InputDrv::BTN_RIGHT_DOWN)
      {
        if(ths.tab_idx < ths.tabs_cnt - 1u) ths.ChangeTab(ths.tab_idx + 1u);
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
// ***   Private: ChangeTab function   *****************************************
// *****************************************************************************
void ProbeScr::ChangeTab(uint8_t tabn)
{
  // Hide old screen
  tab[tab_idx]->Hide();
  // Save scale to control
  tab_idx = tabn;
  // Set new page
  tabs.SetSelectedTab(tab_idx);
  // Show new screen
  tab[tab_idx]->Show();
}

// *****************************************************************************
// ***   Private constructor   *************************************************
// *****************************************************************************
ProbeScr::ProbeScr() : left_btn(Application::GetInstance().GetLeftButton()),
                       right_btn(Application::GetInstance().GetRightButton()) {};

// *****************************************************************************
// *****************************************************************************
// ***   CENTER FINDER TAB   ***************************************************
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
CenterFinderTab& CenterFinderTab::GetInstance()
{
  static CenterFinderTab center_finder_tab;
  return center_finder_tab;
}

// *****************************************************************************
// ***   CenterFinderTab Setup   ***********************************************
// *****************************************************************************
Result CenterFinderTab::Setup(int32_t y, int32_t height)
{
  int32_t start_y = y + BORDER_W;

  // Fill all windows
  for(uint32_t i = 0u; i < grbl_comm.GetLimitedNumberOfAxis(NumberOf(dw_real)); i++)
  {
    // Real position
    dw_real[i].SetParams(BORDER_W + ((display_drv.GetScreenW() - BORDER_W * 4) / 3 + BORDER_W) * i, start_y + BORDER_W*2 + Font_10x18::GetInstance().GetCharH(), (display_drv.GetScreenW() - BORDER_W * 4) / 3, Font_10x18::GetInstance().GetCharH() + Font_6x8::GetInstance().GetCharH()*2 + BORDER_W, 7u, grbl_comm.GetReportUnitsPrecision());
    dw_real[i].SetBorder(BORDER_W / 2, COLOR_GREY);
    dw_real[i].SetBorderColor(COLOR_GREY, COLOR_RED);
    dw_real[i].SetDataFont(Font_10x18::GetInstance());
    dw_real[i].SetNumber(0);
    dw_real[i].SetUnits(grbl_comm.GetReportUnits(), DataWindow::BOTTOM_RIGHT, Font_6x8::GetInstance());
    dw_real[i].SetCallback(AppTask::GetCurrent());
    // Axis Name
    dw_real_name[i].SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_10x18::GetInstance());
    dw_real_name[i].Move(dw_real[i].GetStartX() + (dw_real[i].GetWidth() - dw_real_name[i].GetWidth()) / 2, dw_real[i].GetStartY() - BORDER_W - dw_real_name[i].GetHeight());
  }
  // Set X and Y data window active
  dw_real[GrblComm::AXIS_X].SetActive(true);
  dw_real[GrblComm::AXIS_Y].SetActive(true);
  // Select both data window
  dw_real[GrblComm::AXIS_X].SetSelected(true);
  dw_real[GrblComm::AXIS_Y].SetSelected(true);

  // Buttons to select type of measurement
  inside_btn.SetParams("INSIDE", BORDER_W, dw_real[0].GetEndY() + BORDER_W, (display_drv.GetScreenW() - BORDER_W * 3) / 2, Font_8x12::GetInstance().GetCharH() * 5, true);
  outside_btn.SetParams("OUTSIDE", display_drv.GetScreenW() - inside_btn.GetWidth() - BORDER_W, inside_btn.GetStartY(), inside_btn.GetWidth(), inside_btn.GetHeight(), true);
  inside_btn.SetCallback(AppTask::GetCurrent());
  outside_btn.SetCallback(AppTask::GetCurrent());
  inside_btn.SetPressed(true);

  // Clearance
  dw_clearance.SetParams(BORDER_W, inside_btn.GetEndY() + BORDER_W, display_drv.GetScreenW() - BORDER_W*2, inside_btn.GetHeight(), 15u, grbl_comm.GetReportUnitsPrecision());
  dw_clearance.SetBorder(BORDER_W, COLOR_RED);
  dw_clearance.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_clearance.SetLimits(0, INT32_MAX);
  dw_clearance.SetNumber(0);
  dw_clearance.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  dw_clearance.SetCallback(AppTask::GetCurrent());
  dw_clearance.SetActive(true);
  // Radius caption
  dw_clearance_name.SetParams("CLEARANCE", dw_clearance.GetStartX() + BORDER_W*2, dw_clearance.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // Distance
  dw_distance.SetParams(BORDER_W, dw_clearance.GetEndY() + BORDER_W, display_drv.GetScreenW() - BORDER_W*2, inside_btn.GetHeight(), 15u, grbl_comm.GetReportUnitsPrecision());
  dw_distance.SetBorder(BORDER_W, COLOR_RED);
  dw_distance.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_distance.SetLimits(0, INT32_MAX);
  dw_distance.SetNumber(0);
  dw_distance.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  dw_distance.SetCallback(AppTask::GetCurrent());
  dw_distance.SetActive(true);
  // Radius caption
  dw_distance_name.SetParams("DISTANCE", dw_distance.GetStartX() + BORDER_W*2, dw_distance.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // Button for precise measurement
  precise_btn.SetParams("PRECISE", BORDER_W, dw_distance.GetEndY() + BORDER_W*2, Font_8x12::GetInstance().GetCharW() * 10, Font_8x12::GetInstance().GetCharH() * 5, true);
  precise_btn.SetCallback(AppTask::GetCurrent());

  // For distance data
  for(uint32_t i = 0u; i < NumberOf(data_str); i++)
  {
    data_str[i].SetParams("", precise_btn.GetEndX() + BORDER_W*2, precise_btn.GetStartY() + Font_10x18::GetInstance().GetCharH() * i, COLOR_WHITE, Font_10x18::GetInstance());
  }

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result CenterFinderTab::Show()
{
  // Set Message Box parameters and callback
  msg_box.Setup("Action needed", "Turn probe 180 degrees\nand press continue");

  // Axis data
  for(uint32_t i = 0u; i < grbl_comm.GetLimitedNumberOfAxis(NumberOf(dw_real)); i++)
  {
    dw_real[i].Show(100);
    dw_real_name[i].Show(100);
  }

  // For measurement data
  for(uint32_t i = 0u; i < NumberOf(data_str); i++)
  {
    data_str[i].Show(100);
  }

  // Buttons to select type of measurement
  inside_btn.Show(100);
  outside_btn.Show(100);
  // Show button for precise measurement
  precise_btn.Show(100);

  // Option for outside probing
  if(outside_btn.GetPressed())
  {
    dw_clearance.Show(100);
    dw_clearance_name.Show(100);
    dw_distance.Show(100);
    dw_distance_name.Show(100);
  }

  // Set "Find Center" text to the Left Soft button
  Application::GetInstance().GetLeftButton().SetString("FIND CENTER");

  // Request offsets to show it
  grbl_comm.RequestOffsets();

  // Set encoder callback handler
  InputDrv::GetInstance().AddEncoderCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessEncoderCallback), this, enc_cble);

  // Enable or disable Find button
  if(dw_real[GrblComm::AXIS_X].IsSelected() || dw_real[GrblComm::AXIS_Y].IsSelected())
  {
    // Enable Left Soft button
    Application::GetInstance().GetLeftButton().Enable();
  }
  else
  {
    // Disable Left Soft button
    Application::GetInstance().GetLeftButton().Disable();
  }

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result CenterFinderTab::Hide()
{
  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);

  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].Hide();
    dw_real_name[i].Hide();
  }

  // For measurement data
  for(uint32_t i = 0u; i < NumberOf(data_str); i++)
  {
    data_str[i].Hide();
  }

  // Buttons to select type of measurement
  inside_btn.Hide();
  outside_btn.Hide();
  // Hide button for precise measurement
  precise_btn.Hide();

  // Option for outside probing
  dw_clearance.Hide();
  dw_clearance_name.Hide();
  dw_distance.Hide();
  dw_distance_name.Hide();

  // Enable Left Soft button because we may disable it earlier
  Application::GetInstance().GetLeftButton().Enable();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result CenterFinderTab::TimerExpired(uint32_t interval)
{
  // Not for the return, for check probing result
  Result result = Result::RESULT_OK;

  // Update numbers with current position and position difference
  for(uint32_t i = 0u; i < grbl_comm.GetLimitedNumberOfAxis(NumberOf(dw_real)); i++)
  {
    dw_real[i].SetNumber(grbl_comm.GetAxisPosition(i));
  }

  // Error check - if state isn't IDLE or RUN, we should abort probing sequence
  if((grbl_comm.GetState() != GrblComm::IDLE) && (grbl_comm.GetState() != GrblComm::RUN) && (grbl_comm.GetState() != GrblComm::HOLD))
  {
    // We should abort probing sequence
    ResetProbeSequence();
  }

  // If we send command and this command executed successfully
  if(((cmd_id != 0) && (grbl_comm.GetCmdResult(cmd_id) == GrblComm::Status_OK) && (grbl_comm.IsStatusReceivedAfterCmd(cmd_id))) ||
     ((cmd_id == 0) && (state == PROBE_START)) ||
     ((cmd_id == 0) && (line_state == PROBE_LINE_START)))
  {
    // Check if command done executing
    if(grbl_comm.GetState() == GrblComm::IDLE)
    {
      // Iterations
      if(state == PROBE_START)
      {
        // For measurement data
        for(uint32_t i = 0u; i < NumberOf(data_str); i++)
        {
          data_str[i].SetString("");
        }
        // Restart line sequence
        line_state = PROBE_LINE_START;
        // Change state to measure X min
        state = dw_real[GrblComm::AXIS_X].IsSelected() ? PROBE_X_MIN : PROBE_Y_MIN;
      }
      else if(state == PROBE_X_MIN)
      {
        // If we done probing X min
        if(line_state == PROBE_LINE_CNT)
        {
          // Restart line sequence
          line_state = PROBE_LINE_START;
          // Change state to measure X max
          state = PROBE_X_MAX;
        }
        else
        {
          // Otherwise continue probing sequence for X min
          result = ProbeLineSequence(line_state, GrblComm::AXIS_X, -1, x_safe_pos, x_min_pos);
        }
      }
      else if(state == PROBE_X_MAX)
      {
        if(line_state == PROBE_LINE_CNT) // If we done probing X max
        {
          // Restart line sequence
          line_state = PROBE_LINE_START;
          // Change state
          state = dw_real[GrblComm::AXIS_Y].IsSelected() ? PROBE_Y_MIN : PROBE_DONE;
        }
        else
        {
          // If result is ready, calculate center and store it as safe position
          if(line_state == PROBE_LINE_RESULT_READY)
          {
            x_safe_pos = (x_min_pos + x_max_pos) / 2;
            x_distance = x_max_pos - x_min_pos;
            // For inside we need to add probe ball tip diameter
            if(inside_btn.GetPressed())
            {
              x_distance += grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_BALL_TIP));
            }
            else // For outside we have to subtract probe ball tip diameter
            {
              x_distance -= grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_BALL_TIP));
            }
            // For the second iteration, we have to find difference
            if(iteration == PROBE_ITERATION_SECOND)
            {
              x_safe_pos = (x_first_iteration_pos + x_safe_pos) / 2;
            }
            else // For the first iteration save x_safe_pos
            {
              x_first_iteration_pos = x_safe_pos;
            }
            // Update X measurement string
            char tmp_x[13u] = {0};
            data_str[0u].SetString(data_str_buf[0u], NumberOf(data_str_buf[0u]), "Dx: %s %s", grbl_comm.ValueToStringWithReportScaler(tmp_x, NumberOf(tmp_x), x_distance), grbl_comm.GetReportUnits());
          }
          // Otherwise continue probing sequence for X max
          result = ProbeLineSequence(line_state, GrblComm::AXIS_X, +1, x_safe_pos, x_max_pos);
        }
      }
      else if(state == PROBE_Y_MIN)
      {
        // If we done probing X max
        if(line_state == PROBE_LINE_CNT)
        {
          // Restart line sequence
          line_state = PROBE_LINE_START;
          // Change state to measure Y max
          state = PROBE_Y_MAX;
        }
        else
        {
          // Otherwise continue probing sequence for Y min
          result = ProbeLineSequence(line_state, GrblComm::AXIS_Y, -1, y_safe_pos, y_min_pos);
        }
      }
      else if(state == PROBE_Y_MAX)
      {
        // If we done probing X max
        if(line_state == PROBE_LINE_CNT)
        {
          // Done probing
          state = PROBE_DONE;
        }
        else
        {
          // If result is ready, calculate center and store it as safe position
          if(line_state == PROBE_LINE_RESULT_READY)
          {
            y_safe_pos = (y_min_pos + y_max_pos) / 2;
            y_distance = y_max_pos - y_min_pos;
            // For inside we need to add probe ball tip diameter
            if(inside_btn.GetPressed())
            {
              y_distance += grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_BALL_TIP));
            }
            else // For outside we have to subtract probe ball tip diameter
            {
              y_distance -= grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_BALL_TIP));
            }
            // For the second iteration, we have to find difference
            if(iteration == PROBE_ITERATION_SECOND)
            {
              y_safe_pos = (y_first_iteration_pos + y_safe_pos) / 2;
            }
            else // For the first iteration save x_safe_pos
            {
              y_first_iteration_pos = y_safe_pos;
            }
            // Update Y measurement string
            char tmp_y[13u] = {0};
            data_str[1u].SetString(data_str_buf[1u], NumberOf(data_str_buf[1u]), "Dy: %s %s", grbl_comm.ValueToStringWithReportScaler(tmp_y, NumberOf(tmp_y), y_distance), grbl_comm.GetReportUnits());

            // Update measurement deviation string
            if(dw_real[GrblComm::AXIS_X].IsSelected())
            {
              distance_diviation = abs(x_distance - y_distance);
              char tmp_d[13u] = {0};
              data_str[2u].SetString(data_str_buf[2u], NumberOf(data_str_buf[2u]), "Dd: %s %s", grbl_comm.ValueToStringWithReportScaler(tmp_d, NumberOf(tmp_d), distance_diviation), grbl_comm.GetReportUnits());
            }
          }
          // Otherwise continue probing sequence for Y max
          result = ProbeLineSequence(line_state, GrblComm::AXIS_Y, +1, y_safe_pos, y_max_pos);
        }
      }
      else if(state == PROBE_DONE)
      {
        // If it is not precise measurement or if it second iterations
        if((precise_btn.GetPressed() == false) || (iteration == PROBE_ITERATION_SECOND))
        {
          // Done probing
          ResetProbeSequence();
        }
        else if(precise_btn.GetPressed())
        {
          if(iteration == PROBE_ITERATION_FIRST)
          {
            // Show message box with request
            msg_box.Show(10000u);
            // Set iteration to prevent entering this again
            iteration = PROBE_ITERATION_MSGBOX_SHOW;
          }
        }
        else
        {
          ; // Do nothing, MISRA rule
        }
      }
    }
  }
  // Error check - if command was sent, but not accepted by controller TODO: do we need this?
  else if(cmd_id && (grbl_comm.GetCmdResult(cmd_id) != GrblComm::Status_Cmd_Not_Executed_Yet) && (grbl_comm.GetCmdResult(cmd_id) != GrblComm::Status_OK))
  {
    // Set error to stop sequence
    result = Result::ERR_CANNOT_EXECUTE;
  }
  else
  {
    ; // Do nothing - MISRA rule
  }

  if(result.IsBad())
  {
    // Send Stop command
    grbl_comm.Stop();
    // Stop probing
    ResetProbeSequence();
  }

  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result CenterFinderTab::ProcessCallback(const void* ptr)
{
  // Change anything only if probing isn't started
  if(state == PROBE_CNT)
  {
    // We can start probing only in IDLE state and if probing isn't started yet
    // Check button
    if(ptr == &Application::GetInstance().GetLeftButton())
    {
      if(grbl_comm.GetState() == GrblComm::IDLE)
      {
        // Start probing only if at least one data window is selected
        if(dw_real[GrblComm::AXIS_X].IsSelected() || dw_real[GrblComm::AXIS_Y].IsSelected())
        {
          // Start probe sequence
          state = PROBE_START;
          line_state = PROBE_LINE_START;
          iteration = PROBE_ITERATION_FIRST;
          // Disable screen change
          ProbeScr::GetInstance().DisableScreenChange();
        }
      }
    }
    // X data window
    else if(ptr == &dw_real[GrblComm::AXIS_X])
    {
      dw_real[GrblComm::AXIS_X].SetSelected(!dw_real[GrblComm::AXIS_X].IsSelected());
    }
    // Y data window
    else if(ptr == &dw_real[GrblComm::AXIS_Y])
    {
      dw_real[GrblComm::AXIS_Y].SetSelected(!dw_real[GrblComm::AXIS_Y].IsSelected());
    }
    // Inside button
    else if(ptr == &inside_btn)
    {
      inside_btn.SetPressed(true);
      outside_btn.SetPressed(false);
      // Option for outside probing
      dw_clearance.Hide();
      dw_clearance_name.Hide();
      dw_distance.Hide();
      dw_distance_name.Hide();
    }
    // Outside button
    else if(ptr == &outside_btn)
    {
      outside_btn.SetPressed(true);
      inside_btn.SetPressed(false);
      // Option for outside probing
      dw_clearance.Show(100);
      dw_clearance_name.Show(100);
      dw_distance.Show(100);
      dw_distance_name.Show(100);
    }
    // Distance data window
    else if(ptr == &dw_distance)
    {
      dw_distance.SetSelected(true);
      dw_clearance.SetSelected(false);
    }
    // Clearance data window
    else if(ptr == &dw_clearance)
    {
      dw_clearance.SetSelected(true);
      dw_distance.SetSelected(false);
    }
    // Precise button
    else if(ptr == &precise_btn)
    {
      precise_btn.SetPressed(!precise_btn.GetPressed());
    }
    else
    {
      ; // Do nothing - MISRA rule
    }

    // Enable or disable Find button
    if(dw_real[GrblComm::AXIS_X].IsSelected() || dw_real[GrblComm::AXIS_Y].IsSelected())
    {
      // Enable Left Soft button
      Application::GetInstance().GetLeftButton().Enable();
    }
    else
    {
      // Disable Left Soft button
      Application::GetInstance().GetLeftButton().Disable();
    }
  }
  else
  {
    if(ptr == &msg_box)
    {
      // If "Ok" button pressed and we in correct iteration state
      if((msg_box.GetResult() == Result::RESULT_OK) && (iteration == PROBE_ITERATION_MSGBOX_SHOW))
      {
        // Start probe sequence
        state = PROBE_START;
        line_state = PROBE_LINE_START;
        iteration = PROBE_ITERATION_SECOND;
      }
      else // Cancel button pressed
      {
        // Stop probing
        ResetProbeSequence();
      }
    }
  }

  // Always good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Private: ResetProbeSequence function   ********************************
// *****************************************************************************
void CenterFinderTab::ResetProbeSequence()
{
  // Clear state
  state = PROBE_CNT;
  // Clear line state
  line_state = PROBE_LINE_CNT;
  // Clear precise iteration flag
  iteration = PROBE_ITERATION_CNT;
  // Clear CMD ID
  cmd_id = 0u;
  // Enable screen change
  ProbeScr::GetInstance().EnableScreenChange();
}

// *****************************************************************************
// ***   Private: ProcessEncoderCallback function   ****************************
// *****************************************************************************
Result CenterFinderTab::ProcessEncoderCallback(CenterFinderTab* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    CenterFinderTab& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    // Process it
    if(enc_val != 0)
    {
      // Change clearance
      if(ths.dw_clearance.IsShow() && ths.dw_clearance.IsSelected())
      {
        ths.dw_clearance.SetNumber(ths.dw_clearance.GetNumber() + enc_val * 100);
      }
      // Change distance
      if(ths.dw_distance.IsShow() && ths.dw_distance.IsSelected())
      {
        ths.dw_distance.SetNumber(ths.dw_distance.GetNumber() + enc_val * 100);
      }
    }
    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   ProbeLineSequence function   ******************************************
// *****************************************************************************
Result CenterFinderTab::ProbeLineSequence(probe_line_state_t& state, uint8_t axis, int32_t dir, int32_t& safe_pos, int32_t& measured_pos)
{
  Result result = Result::RESULT_OK;

  if(inside_btn.GetPressed())
  {
    result = ProbeInsideLineSequence(state, axis, dir, safe_pos, measured_pos);
  }
  else
  {
    result = ProbeOutsideLineSequence(state, axis, dir, dw_distance.GetNumber(), dw_clearance.GetNumber(), safe_pos, measured_pos);
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   ProbeInsideLineSequence function   ************************************
// *****************************************************************************
Result CenterFinderTab::ProbeInsideLineSequence(probe_line_state_t& state, uint8_t axis, int32_t dir, int32_t& safe_pos, int32_t& measured_pos)
{
  Result result = Result::RESULT_OK;

  // Iterations
  if(state == PROBE_LINE_START)
  {
    // Absolute mode necessary for probing, otherwise probe can be screwed up
    grbl_comm.SetAbsoluteMode();
    // Get current axis position to return probe back after probing
    safe_pos = grbl_comm.GetAxisPosition(axis);
    // Try to probe axis +/- 1 meter at search feed
    result = grbl_comm.ProbeAxisTowardWorkpiece(axis, grbl_comm.GetAxisPosition(axis) + (grbl_comm.ConvertMetricToUnits(1000000) * dir), grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_SEARCH_FEED)), cmd_id);
    // Set next state
    state = PROBE_LINE_FAST;
  }
  else if(state == PROBE_LINE_FAST)
  {
    // Get probe axis position
    measured_pos = grbl_comm.GetProbePosition(axis);
    // Move away from workpiece at search feed
    result = grbl_comm.ProbeAxisAwayFromWorkpiece(axis, safe_pos, grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_SEARCH_FEED)), cmd_id);
    // Set next state
    state = PROBE_LINE_FAST_RETURN;
  }
  else if(state == PROBE_LINE_FAST_RETURN)
  {
    // Try to probe axis at measured position +/- 1 mm at lock feed
    result = grbl_comm.ProbeAxisTowardWorkpiece(axis, measured_pos + (grbl_comm.ConvertMetricToUnits(1000) * dir), grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_LOCK_FEED)), cmd_id);
    // Set next state
    state = PROBE_LINE_SLOW;
  }
  else if(state == PROBE_LINE_SLOW)
  {
    // Get probe axis position
    measured_pos = grbl_comm.GetProbePosition(axis);
    // Set result ready state
    state = PROBE_LINE_RESULT_READY;
  }
  else if(state == PROBE_LINE_RESULT_READY)
  {
    // Rapid move axis to the point before probing
    result = grbl_comm.MoveAxis(axis, safe_pos, 0u, cmd_id);
    // Return after axis probing
    state = PROBE_LINE_SLOW_RETURN;
  }
  else if(state == PROBE_LINE_SLOW_RETURN)
  {
    // Probe max state - done sequence
    state = PROBE_LINE_CNT;
  }
  else
  {
    ; // Should never get there
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   ProbeOutsideLineSequence function   ***********************************
// *****************************************************************************
Result CenterFinderTab::ProbeOutsideLineSequence(probe_line_state_t& state, uint8_t axis, int32_t dir, int32_t len, int32_t dive, int32_t& safe_pos, int32_t& measured_pos)
{
  Result result = Result::RESULT_OK;

  // Error flag. We don't need return error immediately.
  static bool error = false;
  // Safe Z position
  static int32_t z_safe_pos = 0;

  // ***************************************************************************
  // *** Move line to start position *******************************************
  // ***************************************************************************
  if(state == PROBE_LINE_START)
  {
    // Absolute mode necessary for probing, otherwise probe can be screwed up
    grbl_comm.SetAbsoluteMode();
    // Get current axis positions to return probe back after probing
    safe_pos = grbl_comm.GetAxisPosition(axis);
    z_safe_pos = grbl_comm.GetAxisPosition(GrblComm::AXIS_Z);
    // Clear error flag
    error = false;
    // Rapid move axis to the dive point
    result = grbl_comm.MoveAxis(axis, safe_pos + len * dir, 0u, cmd_id);
    // Set next state
    state = PROBE_LINE_MOVE;
  }
  // ***************************************************************************
  // *** Dive probe to measurement *********************************************
  // ***************************************************************************
  else if(state == PROBE_LINE_MOVE)
  {
    // Dive before probing. We use probing command to avoid probe damage if distance is incorrect.
    result = grbl_comm.ProbeAxisTowardWorkpiece(GrblComm::AXIS_Z, z_safe_pos - grbl_comm.ConvertMetricToUnits(dive), grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_SEARCH_FEED)), cmd_id, false);
    // Set next state
    state = PROBE_LINE_DIVE;
  }
  // ***************************************************************************
  // *** Start fast measurement toward workpiece *******************************
  // ***************************************************************************
  else if(state == PROBE_LINE_DIVE)
  {
    // Check if probe reached dive depth
    if( (grbl_comm.GetAxisPosition(GrblComm::AXIS_Z) != (z_safe_pos - grbl_comm.ConvertMetricToUnits(dive))) ||
        (grbl_comm.IsProbeTriggered() == true) )
    {
      // Set error flag - we need report it at the end of the sequence
      error = true;
      // Error - probe isn't reached requested depth. Lift the probe and return.
      result = grbl_comm.MoveAxis(GrblComm::AXIS_Z, z_safe_pos, 0u, cmd_id);
      // Skip probing sequence
      state = PROBE_LINE_ASCEND;
    }
    else
    {
      // Try to probe axis +/- 1 meter at search feed
      result = grbl_comm.ProbeAxisTowardWorkpiece(axis, grbl_comm.GetAxisPosition(axis) + (grbl_comm.ConvertMetricToUnits(1000000) * (-dir)), grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_SEARCH_FEED)), cmd_id);
      // Set next state
      state = PROBE_LINE_FAST;
    }
  }
  // ***************************************************************************
  // *** Return after fast measurement toward workpiece ************************
  // ***************************************************************************
  else if(state == PROBE_LINE_FAST)
  {
    // Get probe axis position
    measured_pos = grbl_comm.GetProbePosition(axis);
    // Move away from workpiece at search feed
    result = grbl_comm.ProbeAxisAwayFromWorkpiece(axis, safe_pos + len * dir, grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_SEARCH_FEED)), cmd_id);
    // Set next state
    state = PROBE_LINE_FAST_RETURN;
  }
  // ***************************************************************************
  // *** Start slow measurement toward workpiece *******************************
  // ***************************************************************************
  else if(state == PROBE_LINE_FAST_RETURN)
  {
    // Try to probe axis at measured position +/- 1 mm at lock feed
    result = grbl_comm.ProbeAxisTowardWorkpiece(axis, measured_pos + (grbl_comm.ConvertMetricToUnits(1000) * (-dir)), grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_LOCK_FEED)), cmd_id);
    // Set next state
    state = PROBE_LINE_SLOW;
  }
  // ***************************************************************************
  // *** Block to allow recalculate safe position  *****************************
  // ***************************************************************************
  else if(state == PROBE_LINE_SLOW)
  {
    // Get probe axis position
    measured_pos = grbl_comm.GetProbePosition(axis);
    // Set result ready state
    state = PROBE_LINE_RESULT_READY;
  }
  // ***************************************************************************
  // *** Return after slow measurement toward workpiece ************************
  // ***************************************************************************
  else if(state == PROBE_LINE_RESULT_READY)
  {
    // Rapid move axis to the point before probing
    result = grbl_comm.MoveAxis(axis, safe_pos + len * dir, 0u, cmd_id);
    // Return after axis probing
    state = PROBE_LINE_SLOW_RETURN;
  }
  // ***************************************************************************
  // *** Ascend to save Z position *********************************************
  // ***************************************************************************
  else if(state == PROBE_LINE_SLOW_RETURN)
  {
    // Rapid ascend Z axis
    result = grbl_comm.MoveAxis(GrblComm::AXIS_Z, z_safe_pos, 0u, cmd_id);
    // Current state
    state = PROBE_LINE_ASCEND;
  }
  // ***************************************************************************
  // *** Return to save position ***********************************************
  // ***************************************************************************
  else if(state == PROBE_LINE_ASCEND)
  {
    // Check if we actually lifted Z axis
    if(z_safe_pos == grbl_comm.GetAxisPosition(GrblComm::AXIS_Z))
    {
      // Rapid move axis to the point before probing
      result = grbl_comm.MoveAxis(axis, safe_pos, 0u, cmd_id);
    }
    else
    {
      // We didn't lifted Z by some reason
      result = Result::ERR_CANNOT_EXECUTE;
    }
    // Probe max state - done sequence
    state = PROBE_LINE_RETURN;
  }
  // ***************************************************************************
  // *** End of sequence *******************************************************
  // ***************************************************************************
  else if(state == PROBE_LINE_RETURN)
  {
    // Set error
    if(error)
    {
      result = Result::ERR_CANNOT_EXECUTE;
    }
    // Probe max state - done sequence
    state = PROBE_LINE_CNT;
  }
  else
  {
    ; // Should never get there
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   CenterFinderTab: Private constructor   ********************************
// *****************************************************************************
CenterFinderTab::CenterFinderTab() : msg_box(Application::GetInstance().GetMsgBox()) {};

// *****************************************************************************
// *****************************************************************************
// ***   EDGE FINDER TAB   *****************************************************
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
EdgeFinderTab& EdgeFinderTab::GetInstance()
{
  static EdgeFinderTab edge_finder_tab;
  return edge_finder_tab;
}

// *****************************************************************************
// ***   EdgeFinderTab Setup   *************************************************
// *****************************************************************************
Result EdgeFinderTab::Setup(int32_t y, int32_t height)
{
  int32_t start_y = y + BORDER_W;

  // Fill all windows
  for(uint32_t i = 0u; i < grbl_comm.GetLimitedNumberOfAxis(NumberOf(dw_real)); i++)
  {
    // Real position
    dw_real[i].SetParams(BORDER_W + ((display_drv.GetScreenW() - BORDER_W * 4) / 3 + BORDER_W) * i, start_y + BORDER_W*2 + Font_10x18::GetInstance().GetCharH(), (display_drv.GetScreenW() - BORDER_W * 4) / 3, Font_10x18::GetInstance().GetCharH() + Font_6x8::GetInstance().GetCharH()*2 + BORDER_W, 7u, grbl_comm.GetReportUnitsPrecision());
    dw_real[i].SetBorder(BORDER_W / 2, COLOR_GREY);
    dw_real[i].SetBorderColor(COLOR_GREY, COLOR_RED);
    dw_real[i].SetDataFont(Font_10x18::GetInstance());
    dw_real[i].SetNumber(0);
    dw_real[i].SetUnits(grbl_comm.GetReportUnits(), DataWindow::BOTTOM_RIGHT, Font_6x8::GetInstance());
    dw_real[i].SetCallback(AppTask::GetCurrent());
    dw_real[i].SetActive(true);
    dw_real[i].SetSelected(false);
    // Axis Name
    dw_real_name[i].SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_10x18::GetInstance());
    dw_real_name[i].Move(dw_real[i].GetStartX() + (dw_real[i].GetWidth() - dw_real_name[i].GetWidth()) / 2, dw_real[i].GetStartY() - BORDER_W - dw_real_name[i].GetHeight());
  }
  // Select X data window
  dw_real[GrblComm::AXIS_X].SetSelected(true);

  // Buttons to select type of measurement
  minus_btn.SetParams("-", BORDER_W, dw_real[0].GetEndY() + BORDER_W, (display_drv.GetScreenW() - BORDER_W * 3) / 2, Font_8x12::GetInstance().GetCharH() * 5, true);
  plus_btn.SetParams("+", display_drv.GetScreenW() - minus_btn.GetWidth() - BORDER_W, minus_btn.GetStartY(), minus_btn.GetWidth(), minus_btn.GetHeight(), true);
  minus_btn.SetCallback(AppTask::GetCurrent());
  plus_btn.SetCallback(AppTask::GetCurrent());
  minus_btn.SetPressed(true);

  // Clearance
  dw_clearance.SetParams(BORDER_W, minus_btn.GetEndY() + BORDER_W, display_drv.GetScreenW() - BORDER_W*2, minus_btn.GetHeight(), 15u, grbl_comm.GetReportUnitsPrecision());
  dw_clearance.SetBorder(BORDER_W, COLOR_RED);
  dw_clearance.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_clearance.SetLimits(0, INT32_MAX);
  dw_clearance.SetNumber(5000); // 5 mm default
  dw_clearance.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  dw_clearance.SetCallback(AppTask::GetCurrent());
  dw_clearance.SetActive(true);
  // Radius caption
  dw_clearance_name.SetParams("CLEARANCE", dw_clearance.GetStartX() + BORDER_W*2, dw_clearance.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // Distance
  dw_tip_diameter.SetParams(BORDER_W, dw_clearance.GetEndY() + BORDER_W, display_drv.GetScreenW() - BORDER_W*2, minus_btn.GetHeight(), 15u, grbl_comm.GetReportUnitsPrecision());
  dw_tip_diameter.SetBorder(BORDER_W, COLOR_RED);
  dw_tip_diameter.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_tip_diameter.SetLimits(0, INT32_MAX);
  dw_tip_diameter.SetNumber(NVM::GetInstance().GetValue(NVM::PROBE_BALL_TIP));
  dw_tip_diameter.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  dw_tip_diameter.SetCallback(AppTask::GetCurrent());
  dw_tip_diameter.SetActive(true);
  // Radius caption
  dw_tip_diameter_name.SetParams("TIP DIAMETER", dw_tip_diameter.GetStartX() + BORDER_W*2, dw_tip_diameter.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // Button for precise measurement
  precise_btn.SetParams("PRECISE", BORDER_W, dw_tip_diameter.GetEndY() + BORDER_W*2, Font_8x12::GetInstance().GetCharW() * 10, Font_8x12::GetInstance().GetCharH() * 5, true);
  precise_btn.SetCallback(AppTask::GetCurrent());

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result EdgeFinderTab::Show()
{
  // Set Message Box parameters and callback
  msg_box.Setup("Action needed", "Turn probe 180 degrees\nand press continue");

  // Axis data
  for(uint32_t i = 0u; i < grbl_comm.GetLimitedNumberOfAxis(NumberOf(dw_real)); i++)
  {
    dw_real[i].Show(100);
    dw_real_name[i].Show(100);
  }

  // Buttons to select type of measurement
  minus_btn.Show(100);
  plus_btn.Show(100);

  // Buttons to enable/disable measurement
  precise_btn.Show(100);

  // Clearance window
  dw_clearance.Show(100);
  dw_clearance_name.Show(100);
  // Tip diameter window
  dw_tip_diameter.SetNumber(NVM::GetInstance().GetValue(NVM::PROBE_BALL_TIP));
  dw_tip_diameter.Show(100);
  dw_tip_diameter_name.Show(100);

  // Set "Find Edge" text to the Left Soft button
  Application::GetInstance().GetLeftButton().SetString("FIND EDGE");

  // Request offsets to show it
  grbl_comm.RequestOffsets();

  // Set encoder callback handler
  InputDrv::GetInstance().AddEncoderCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessEncoderCallback), this, enc_cble);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result EdgeFinderTab::Hide()
{
  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);

  // Axis data
  for(uint32_t i = 0u; i < grbl_comm.GetLimitedNumberOfAxis(NumberOf(dw_real)); i++)
  {
    dw_real[i].Hide();
    dw_real_name[i].Hide();
  }

  // Buttons to select type of measurement
  minus_btn.Hide();
  plus_btn.Hide();

  // Buttons to enable/disable measurement
  precise_btn.Hide();

  // Option for outside probing
  dw_clearance.Hide();
  dw_clearance_name.Hide();
  dw_tip_diameter.Hide();
  dw_tip_diameter_name.Hide();

  // To update probe tip diameter
  NVM::GetInstance().WriteData();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result EdgeFinderTab::TimerExpired(uint32_t interval)
{
  // Not for the return, for check probing result
  Result result = Result::RESULT_OK;

  // Update numbers with current position and position difference
  for(uint32_t i = 0u; i < grbl_comm.GetLimitedNumberOfAxis(NumberOf(dw_real)); i++)
  {
    dw_real[i].SetNumber(grbl_comm.GetAxisPosition(i));
  }

  // Error check - if state isn't IDLE or RUN
  if((grbl_comm.GetState() != GrblComm::IDLE) && (grbl_comm.GetState() != GrblComm::RUN) && (grbl_comm.GetState() != GrblComm::HOLD))
  {
    // We should abort probing sequence
    ResetProbeSequence();
  }

  // If we send command and this command executed successfully
  if(((cmd_id != 0) && (grbl_comm.GetCmdResult(cmd_id) == GrblComm::Status_OK) && (grbl_comm.IsStatusReceivedAfterCmd(cmd_id))) ||
     ((cmd_id == 0) && (state == PROBE_START)))
  {
    // Check if command done executing
    if(grbl_comm.GetState() == GrblComm::IDLE)
    {
      // Iterations
      if(state == PROBE_START)
      {
        // Set probing axis
        axis = dw_real[GrblComm::AXIS_X].IsSelected() ? GrblComm::AXIS_X : (dw_real[GrblComm::AXIS_Y].IsSelected() ? GrblComm::AXIS_Y : GrblComm::AXIS_Z);
        // If minus button pressed - direction is negative, otherwise positive
        dir = minus_btn.GetPressed() ? -1 : +1; // TODO: Z axis can be probed only to - direction
        // Absolute mode necessary for probing, otherwise probe can be screwed up
        grbl_comm.SetAbsoluteMode();
        // Get current axis position to return probe back after probing
        safe_pos = grbl_comm.GetAxisPosition(axis);
        // Try to probe axis +/- 1 meter at search feed
        result = grbl_comm.ProbeAxisTowardWorkpiece(axis, grbl_comm.GetAxisPosition(axis) + (grbl_comm.ConvertMetricToUnits(1000000) * dir), grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_SEARCH_FEED)), cmd_id);
        // Set next state
        state = PROBE_FAST;
      }
      else if(state == PROBE_FAST)
      {
        // Get probe axis position
        measured_pos = grbl_comm.GetProbePosition(axis);
        // Move away from workpiece at search feed
        result = grbl_comm.ProbeAxisAwayFromWorkpiece(axis, safe_pos, grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_SEARCH_FEED)), cmd_id);
        // Set next state
        state = PROBE_FAST_RETURN;
      }
      else if(state == PROBE_FAST_RETURN)
      {
        // Try to probe axis at measured position +/- 1 mm at lock feed
        result = grbl_comm.ProbeAxisTowardWorkpiece(axis, measured_pos + (grbl_comm.ConvertMetricToUnits(1000) * dir), grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_LOCK_FEED)), cmd_id);
        // Set next state
        state = PROBE_SLOW;
      }
      else if(state == PROBE_SLOW)
      {
        // Get probe axis position
        measured_pos = grbl_comm.GetProbePosition(axis);
        // Save position for first iteration
        if(iteration == PROBE_ITERATION_SECOND)
        {
          measured_pos = (first_iteration_measured_pos + measured_pos) / 2;
        }
        else
        {
          first_iteration_measured_pos = measured_pos;
        }
        // Move away from workpiece at search feed
        result = grbl_comm.ProbeAxisAwayFromWorkpiece(axis, safe_pos, grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_SEARCH_FEED)), cmd_id);
        // Return after axis probing
        state = PROBE_SLOW_RETURN;
      }
      // ***********************************************************************
      // *** Ascend to save Z position *****************************************
      // ***********************************************************************
      else if(state == PROBE_SLOW_RETURN)
      {
        if(!precise_btn.GetPressed() || (axis == GrblComm::AXIS_Z) || (iteration == PROBE_ITERATION_SECOND))
        {
          // For Z axis we need lift Z axis precisely Clearance from measured position
          // and we don't need move axis after that, so we need to skip next step
          if(axis == GrblComm::AXIS_Z)
          {
            // Rapid ascend Z axis
            result = grbl_comm.MoveAxis(GrblComm::AXIS_Z, measured_pos + dw_clearance.GetNumber(), 0u, cmd_id);
            // Skip next step
            state = PROBE_RETURN;
          }
          else
          {
            // Rapid ascend Z axis
            result = grbl_comm.MoveAxis(GrblComm::AXIS_Z, grbl_comm.GetAxisPosition(GrblComm::AXIS_Z) + dw_clearance.GetNumber(), 0u, cmd_id);
            // Current state
            state = PROBE_ASCEND;
          }
        }
        else if(precise_btn.GetPressed() && (iteration == PROBE_ITERATION_FIRST))
        {
          // Show message box with request
          msg_box.Show(10000u);
          // Set iteration to prevent entering this again
          iteration = PROBE_ITERATION_MSGBOX_SHOW;
        }
        else
        {
          ; // Do nothing - MISRA rule
        }
      }
      // ***************************************************************************
      // *** Return to save position ***********************************************
      // ***************************************************************************
      else if(state == PROBE_ASCEND)
      {
        // Use probe command to move axis over the edge. Strict flag set to false
        // because we don't expect probe to be triggered, we using it as safety
        // to prevent probe damage in case clearance not big enough.
        result = grbl_comm.ProbeAxisTowardWorkpiece(axis, measured_pos + (dw_tip_diameter.GetNumber() / 2 * dir), grbl_comm.ConvertMetricToUnits(NVM::GetInstance().GetValue(NVM::PROBE_SEARCH_FEED)), cmd_id, false);
        // Probe max state - done sequence
        state = PROBE_RETURN;
      }
      // ***************************************************************************
      // *** End of sequence *******************************************************
      // ***************************************************************************
      else if(state == PROBE_RETURN)
      {
        // Done probing
        ResetProbeSequence();
      }
      else
      {
        ; // Do nothing - MISRA rule
      }
    }
  }
  // Error check - if command was sent, but not accepted by controller TODO: do we need this?
  else if(cmd_id && (grbl_comm.GetCmdResult(cmd_id) != GrblComm::Status_Cmd_Not_Executed_Yet) && (grbl_comm.GetCmdResult(cmd_id) != GrblComm::Status_OK))
  {
    // Set error to stop sequence
    result = Result::ERR_CANNOT_EXECUTE;
  }
  else
  {
    ; // Do nothing - MISRA rule
  }

  if(result.IsBad())
  {
    // Send Stop command
    grbl_comm.Stop();
    // Clear state
    ResetProbeSequence();
  }

  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result EdgeFinderTab::ProcessCallback(const void* ptr)
{
  // Change anything only if probing isn't started
  if(state == PROBE_CNT)
  {
    // We can start probing only in IDLE state and if probing isn't started yet
    // Check button
    if(ptr == &Application::GetInstance().GetLeftButton())
    {
      if(grbl_comm.GetState() == GrblComm::IDLE)
      {
        // Start probing sequence
        state = PROBE_START;
        // Set first iteration
        iteration = PROBE_ITERATION_FIRST;
        // Disable screen change
        ProbeScr::GetInstance().DisableScreenChange();
      }
    }
    // X data window
    else if(ptr == &dw_real[GrblComm::AXIS_X])
    {
      dw_real[GrblComm::AXIS_X].SetSelected(true);
      dw_real[GrblComm::AXIS_Y].SetSelected(false);
      dw_real[GrblComm::AXIS_Z].SetSelected(false);
      precise_btn.Enable();
      plus_btn.Enable();
    }
    // Y data window
    else if(ptr == &dw_real[GrblComm::AXIS_Y])
    {
      dw_real[GrblComm::AXIS_Y].SetSelected(true);
      dw_real[GrblComm::AXIS_X].SetSelected(false);
      dw_real[GrblComm::AXIS_Z].SetSelected(false);
      precise_btn.Enable();
      plus_btn.Enable();
    }
    // Z data window
    else if(ptr == &dw_real[GrblComm::AXIS_Z])
    {
      dw_real[GrblComm::AXIS_Z].SetSelected(true);
      dw_real[GrblComm::AXIS_X].SetSelected(false);
      dw_real[GrblComm::AXIS_Y].SetSelected(false);
      // Precise button can't be used with Z axis
      precise_btn.Disable();
      // Z probe can go only to negative direction
      plus_btn.Disable();
      ProcessCallback(&minus_btn);
    }
    // Minus button
    else if(ptr == &minus_btn)
    {
      minus_btn.SetPressed(true);
      plus_btn.SetPressed(false);
    }
    // Plus button
    else if(ptr == &plus_btn)
    {
      plus_btn.SetPressed(true);
      minus_btn.SetPressed(false);
    }
    // Distance data window
    else if(ptr == &dw_tip_diameter)
    {
      dw_tip_diameter.SetSelected(true);
      dw_clearance.SetSelected(false);
    }
    // Clearance data window
    else if(ptr == &dw_clearance)
    {
      dw_clearance.SetSelected(true);
      dw_tip_diameter.SetSelected(false);
    }
    // Precise button
    else if(ptr == &precise_btn)
    {
      precise_btn.SetPressed(!precise_btn.GetPressed());
    }
    else
    {
      ; // Do nothing - MISRA rule
    }
  }
  else
  {
    if(ptr == &msg_box)
    {
      // If "Ok" button pressed and we in correct iteration state
      if((msg_box.GetResult() == Result::RESULT_OK) && (iteration == PROBE_ITERATION_MSGBOX_SHOW))
      {
        // Start probe sequence
        state = PROBE_START;
        iteration = PROBE_ITERATION_SECOND;
      }
      else // Cancel button pressed
      {
        // Stop probing
        ResetProbeSequence();
      }
    }
  }

  // Always good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Private: ResetProbeSequence function   ********************************
// *****************************************************************************
void EdgeFinderTab::ResetProbeSequence()
{
  // Clear state
  state = PROBE_CNT;
  // Clear iteration
  iteration = PROBE_ITERATION_CNT;
  // Clear CMD ID
  cmd_id = 0u;
  // Enable screen change
  ProbeScr::GetInstance().EnableScreenChange();
}

// *************************************************************************
// ***   Private: ProcessEncoderCallback function   ************************
// *************************************************************************
Result EdgeFinderTab::ProcessEncoderCallback(EdgeFinderTab* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    EdgeFinderTab& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    // Process it
    if(enc_val != 0)
    {
      // Change clearance
      if(ths.dw_clearance.IsSelected())
      {
        ths.dw_clearance.SetNumber(ths.dw_clearance.GetNumber() + enc_val * 100);
      }
      // Change distance
      if(ths.dw_tip_diameter.IsSelected())
      {
        ths.dw_tip_diameter.SetNumber(ths.dw_tip_diameter.GetNumber() + enc_val * 100);
        NVM::GetInstance().SetValue(NVM::PROBE_BALL_TIP, ths.dw_tip_diameter.GetNumber());
      }
    }
    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   EdgeFinderTab: Private constructor   **********************************
// *****************************************************************************
EdgeFinderTab::EdgeFinderTab() : msg_box(Application::GetInstance().GetMsgBox()) {};

// *****************************************************************************
// *****************************************************************************
// ***   TOOL OFFSET TAB   *****************************************************
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
ToolOffsetTab& ToolOffsetTab::GetInstance()
{
  static ToolOffsetTab tool_offset_tab;
  return tool_offset_tab;
}

// *****************************************************************************
// ***   ToolOffsetTab Setup   *************************************************
// *****************************************************************************
Result ToolOffsetTab::Setup(int32_t y, int32_t height)
{
  int32_t start_y = y + BORDER_W;
  // Data window height
  uint32_t window_height = Font_8x12::GetInstance().GetCharH() * 5u;

  // Tool offset position
  dw_tool.SetParams(BORDER_W, start_y + BORDER_W, display_drv.GetScreenW() - BORDER_W*2,  window_height, 7u, grbl_comm.GetReportUnitsPrecision());
  dw_tool.SetBorder(BORDER_W, COLOR_BLUE);
  dw_tool.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_tool.SetNumber(grbl_comm.GetToolLengthOffset()); // Get current position at startup
  dw_tool.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  dw_tool.SetCallback(AppTask::GetCurrent());
  dw_tool.SetActive(false);
  // Measure offset button
  get_offset_btn.SetParams("MEASURE OFFSET", BORDER_W, dw_tool.GetEndY() + BORDER_W*2, (display_drv.GetScreenW() - BORDER_W*3) / 2, Font_8x12::GetInstance().GetCharH() * 5, true);
  get_offset_btn.SetCallback(AppTask::GetCurrent());
  // Clear offset button
  clear_offset_btn.SetParams("CLEAR OFFSET", get_offset_btn.GetEndX() + BORDER_W, dw_tool.GetEndY() + BORDER_W*2, (display_drv.GetScreenW() - BORDER_W*3) / 2, Font_8x12::GetInstance().GetCharH() * 5, true);
  clear_offset_btn.SetCallback(AppTask::GetCurrent());

  // Tool offset Name
  name_base.SetParams("BASE POSITION", 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
  name_base.Move((display_drv.GetScreenW()  / 2) - (name_base.GetWidth() / 2), get_offset_btn.GetEndY() + BORDER_W * 2u);
  // Tool offset position
  dw_base.SetParams(BORDER_W, name_base.GetEndY() + BORDER_W, display_drv.GetScreenW() - BORDER_W*2,  window_height, 7u, grbl_comm.GetReportUnitsPrecision());
  dw_base.SetBorder(BORDER_W, COLOR_MAGENTA);
  dw_base.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_base.SetNumber(0); // Get current position at startup
  dw_base.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  dw_base.SetCallback(AppTask::GetCurrent());
  dw_base.SetActive(false);
  // Measure offset button
  get_base_btn.SetParams("MEASURE BASE", BORDER_W, dw_base.GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2, Font_8x12::GetInstance().GetCharH() * 5, true);
  get_base_btn.SetCallback(AppTask::GetCurrent());

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result ToolOffsetTab::Show()
{
  // Tool offset window and name
  name_tool.Show(100);
  dw_tool.Show(100);
  // Base position window and name
  name_base.Show(100);
  dw_base.Show(100);

  // Fill all windows
  for(uint32_t i = 0u; i < grbl_comm.GetLimitedNumberOfAxis(3u); i++)
  {
    DataWindow& dw_real = Application::GetInstance().GetRealDataWindow(i);
    String& dw_real_name = Application::GetInstance().GetRealDataWindowNameString(i);

    // Real position
    dw_real.SetParams(BORDER_W + ((display_drv.GetScreenW() - BORDER_W * 4) / 3 + BORDER_W) * i, Application::GetInstance().GetScreenEndY() - Font_8x12::GetInstance().GetCharH() * 2u - BORDER_W, (display_drv.GetScreenW() - BORDER_W * 4) / 3, Font_8x12::GetInstance().GetCharH() * 2u, 7u, grbl_comm.GetReportUnitsPrecision());
    dw_real.SetBorder(BORDER_W / 2, COLOR_GREY);
    dw_real.SetDataFont(Font_8x12::GetInstance());
    dw_real.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT, Font_6x8::GetInstance());
    // Axis Name
    dw_real_name.SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_6x8::GetInstance());
    dw_real_name.Move(dw_real.GetStartX() + BORDER_W, dw_real.GetStartY() + BORDER_W);

    dw_real.Show(100);
    dw_real_name.Show(100);
  }

  // Measure offset button
  get_offset_btn.Show(102);
  // Clear offset button
  clear_offset_btn.Show(102);
  // Measure base button
  get_base_btn.Show(102);

  // Request offsets to show it
  grbl_comm.RequestOffsets();

  // Hide Left Soft button since it doesn't used on this screen
  Application::GetInstance().GetLeftButton().Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result ToolOffsetTab::Hide()
{
  // Tool offset window and name
  name_tool.Hide();
  dw_tool.Hide();

  // Base position window and name
  name_base.Hide();
  dw_base.Hide();

  // Axis data
  for(uint32_t i = 0u; i < GrblComm::AXIS_CNT; i++)
  {
    Application::GetInstance().GetRealDataWindow(i).Hide();
    Application::GetInstance().GetRealDataWindowNameString(i).Hide();
  }

  // Measure offset button
  get_offset_btn.Hide();
  // Clear offset button
  clear_offset_btn.Hide();
  // Measure base button
  get_base_btn.Hide();

  // Show Left Soft button because we hided it earlier
  Application::GetInstance().GetLeftButton().Show();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result ToolOffsetTab::TimerExpired(uint32_t interval)
{
  Result result = Result::RESULT_OK;

  // Set actual tool offset
  dw_tool.SetNumber(grbl_comm.GetToolLengthOffset());

  // If we in probing cycle
  if(state != PROBE_CNT)
  {
    // No command sent yet - start probing
    if(cmd_id == 0u)
    {
      // Try to probe Z axis -1 meter at feed 100 mm/min
      result = grbl_comm.ProbeAxisTowardWorkpiece(GrblComm::AXIS_Z, grbl_comm.GetAxisPosition(GrblComm::AXIS_Z) - grbl_comm.ConvertMetricToUnits(1000000), grbl_comm.ConvertMetricToUnits(100u), cmd_id);
      // Check result
      if(result == Result::ERR_CANNOT_EXECUTE)
      {
        // Clear cmd id
        cmd_id = 0u;
        // Clear state
        state = PROBE_CNT;
        // Enable screen change
        ProbeScr::GetInstance().EnableScreenChange();
        // Set ok result since we cancelled probing
        result = Result::RESULT_OK;
      }
    }
    // If we send probing command and this command executed successfully
    else if((grbl_comm.GetCmdResult(cmd_id) == GrblComm::Status_OK) && (grbl_comm.IsStatusReceivedAfterCmd(cmd_id)) && (grbl_comm.GetState() == GrblComm::IDLE))
    {
      // Get probe Z position
      int32_t probe_pos = grbl_comm.GetProbeMachinePosition(GrblComm::AXIS_Z);
      // If we tried base - update base data window
      if(state == PROBE_BASE) dw_base.SetNumber(probe_pos);
      else if(state == PROBE_TOOL) // If tried tool
      {
        // Calculate difference between base and tool
        int32_t tool_diff = probe_pos - dw_base.GetNumber();
        // Set tool offset
        result = grbl_comm.SetToolLengthOffset(tool_diff);
        // Request offsets to update it on the display
        if(result.IsGood()) grbl_comm.RequestOffsets();
      }
      else
      {
        ; // Do nothing
      }
      if(result.IsGood())
      {
        // Move Z axis to the point before probing at feed 500 mm/min
        grbl_comm.JogInMachineCoodinates(GrblComm::AXIS_Z, z_position, grbl_comm.ConvertMetricToUnits(500u));
        // Clear cmd
        cmd_id = 0u;
        // Clear state
        state = PROBE_CNT;
        // Enable screen change
        ProbeScr::GetInstance().EnableScreenChange();
      }
    }
    // Error check - if command was sent, but not accepted by controller TODO: do we need this?
    else if(cmd_id && (grbl_comm.GetCmdResult(cmd_id) != GrblComm::Status_Cmd_Not_Executed_Yet) && (grbl_comm.GetCmdResult(cmd_id) != GrblComm::Status_OK))
    {
      // Set error to stop sequence
      result = Result::ERR_CANNOT_EXECUTE;
    }
    else
    {
      ; // Do nothing
    }
  }

  if(result.IsBad())
  {
    // Send Stop command
    grbl_comm.Stop();
    // Clear cmd
    cmd_id = 0u;
    // Clear state
    state = PROBE_CNT;
    // Enable screen change
    ProbeScr::GetInstance().EnableScreenChange();
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result ToolOffsetTab::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  // We can start probing only in IDLE state and if probing isn't started yet
  if((grbl_comm.GetState() == GrblComm::IDLE) && (state == PROBE_CNT))
  {
    // Check buttons
    if((ptr == &get_base_btn) || (ptr == &get_offset_btn) || (ptr == &clear_offset_btn))
    {
      // Get current Z position to restore it after tool length offset clear and to return Z axis back after probing
      int32_t z_pos = grbl_comm.GetAxisPosition(GrblComm::AXIS_Z);
      // Clear tool length offset
      result |= grbl_comm.ClearToolLengthOffset();
      // Request offsets to show it
      result |= grbl_comm.RequestOffsets();
      // Restore axis position
      result |= grbl_comm.SetAxisPosition(GrblComm::AXIS_Z, z_pos);
      // Set state for probing only if result is good for previous commands
      if(result.IsGood() && ((ptr == &get_base_btn) || (ptr == &get_offset_btn)))
      {
        // Get current Z position to return Z axis back after probing
        z_position = grbl_comm.GetAxisMachinePosition(GrblComm::AXIS_Z);
        // Set current state
        if(ptr == &get_base_btn) state = PROBE_BASE;
        else                     state = PROBE_TOOL;
        // Disable screen change
        ProbeScr::GetInstance().DisableScreenChange();
      }
    }
  }

  // Always good
  return Result::RESULT_OK;
}
