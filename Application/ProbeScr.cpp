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
  tabs.SetText(tabs_cnt, "Tool", "Offset", Font_10x18::GetInstance());
  tab[tabs_cnt++] = &ToolOffsetTab::GetInstance();
  tabs.SetText(tabs_cnt, "Center", "Finder", Font_10x18::GetInstance());
  tab[tabs_cnt++] = &CenterFinderTab::GetInstance();
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

  // Stop button
  right_btn.SetParams("Stop", display_drv.GetScreenW() - display_drv.GetScreenW() / 2 + BORDER_W, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3, display_drv.GetScreenW() / 2 - BORDER_W, Font_8x12::GetInstance().GetCharH() * 3, true);
  right_btn.SetCallback(AppTask::GetCurrent());

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

  // Stop button
  right_btn.Show(102);

  // Set callback handler for left and right buttons
  InputDrv::GetInstance().AddButtonsCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this, InputDrv::BTNM_LEFT | InputDrv::BTNM_RIGHT | InputDrv::BTNM_LEFT_DOWN | InputDrv::BTNM_RIGHT_DOWN, btn_cble);

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

  // Reset button
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
  if(ptr == &right_btn)
  {
    grbl_comm.Stop(); // Send Stop command
  }
  else if(ptr == &tabs) // Change tab when another tab selected
  {
    ChangeTab(tabs.GetSelectedTab());
  }
  else
  {
    tab[tab_idx]->ProcessCallback(ptr);
  }

  // Always good
  return Result::RESULT_OK;
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

    // Right button - Send Stop command to GRBL
    if(btn.btn == InputDrv::BTN_RIGHT)
    {
      // If button pressed
      if(btn.state == true)
      {
        // Press "Reset" button on the screen
        ths.right_btn.SetPressed(true);
      }
      else // Released
      {
        // Release "Reset" button on the screen
        ths.right_btn.SetPressed(false);
        // And call callback
        ths.ProcessCallback(&ths.right_btn);
      }
    }
    if(btn.btn == InputDrv::BTN_LEFT_DOWN)
    {
      if(ths.tab_idx > 0u) ths.ChangeTab(ths.tab_idx - 1u);
    }
    else if(btn.btn == InputDrv::BTN_RIGHT_DOWN)
    {
      if(ths.tab_idx < ths.tab_idx - 1u) ths.ChangeTab(ths.tab_idx + 1u);
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
  dw_tool.SetParams(BORDER_W, start_y + BORDER_W, display_drv.GetScreenW() - BORDER_W*2,  window_height, 4u, 3u);
  dw_tool.SetBorder(BORDER_W, COLOR_BLUE);
  dw_tool.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_tool.SetNumber(grbl_comm.GetToolLengthOffset()); // Get current position at startup
  dw_tool.SetUnits("mm", DataWindow::RIGHT);
  dw_tool.SetCallback(AppTask::GetCurrent());
  dw_tool.SetActive(false);
  // Measure offset button
  get_offset_btn.SetParams("MEASURE OFFSET", BORDER_W, dw_tool.GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2, Font_8x12::GetInstance().GetCharH() * 5, true);
  get_offset_btn.SetCallback(AppTask::GetCurrent());

  // Tool offset Name
  name_base.SetParams("BASE POSITION", 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
  name_base.Move((display_drv.GetScreenW()  / 2) - (name_base.GetWidth() / 2), get_offset_btn.GetEndY() + BORDER_W * 2u);
  // Tool offset position
  dw_base.SetParams(BORDER_W, name_base.GetEndY() + BORDER_W, display_drv.GetScreenW() - BORDER_W*2,  window_height, 4u, 3u);
  dw_base.SetBorder(BORDER_W, COLOR_MAGENTA);
  dw_base.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_base.SetNumber(0); // Get current position at startup
  dw_base.SetUnits("mm", DataWindow::RIGHT);
  dw_base.SetCallback(AppTask::GetCurrent());
  dw_base.SetActive(false);
  // Measure offset button
  get_base_btn.SetParams("MEASURE BASE", BORDER_W, dw_base.GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2, Font_8x12::GetInstance().GetCharH() * 5, true);
  get_base_btn.SetCallback(AppTask::GetCurrent());

  // Fill all windows
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    // Real position
    dw_real[i].SetParams(BORDER_W + ((display_drv.GetScreenW() - BORDER_W * 4) / 3 + BORDER_W) * i, get_base_btn.GetEndY() + BORDER_W, (display_drv.GetScreenW() - BORDER_W * 4) / 3, (window_height - BORDER_W) / 2, 4u, 3u);
    dw_real[i].SetBorder(BORDER_W / 2, COLOR_GREY);
    dw_real[i].SetDataFont(Font_8x12::GetInstance());
    dw_real[i].SetNumber(0);
    dw_real[i].SetUnits("mm", DataWindow::RIGHT, Font_6x8::GetInstance());
    // Axis Name
    dw_real_name[i].SetParams(grbl_comm.GetAxisName(i), 0, 0, COLOR_WHITE, Font_6x8::GetInstance());
    dw_real_name[i].Move(dw_real[i].GetStartX() + BORDER_W, dw_real[i].GetStartY() + BORDER_W);
  }

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

  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].Show(100);
    dw_real_name[i].Show(100);
  }

  // Go button
  get_offset_btn.Show(102);
  // Reset button
  get_base_btn.Show(102);

  // Request offsets to show it
  grbl_comm.RequestOffsets();

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
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].Hide();
    dw_real_name[i].Hide();
  }

  // Go button
  get_offset_btn.Hide();
  // Reset button
  get_base_btn.Hide();

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

  // Update numbers with current position and position difference
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].SetNumber(grbl_comm.GetAxisPosition(i));
  }

  // If we send command and this command executed successfully
  if(cmd_id && (grbl_comm.GetCmdResult(cmd_id) == GrblComm::Status_OK))
  {
    // Check if command end executing
    if(grbl_comm.GetState() == GrblComm::IDLE)
    {
      // Get probe Z position
      int32_t probe_pos = grbl_comm.GetProbeMachinePosition(GrblComm::AXIS_Z);
      // If we tried base - update base data window
      if(state == PROBE_BASE) dw_base.SetNumber(probe_pos);
      else if(state == PROBE_TOOL) // If tried tool
      {
        // Calculate difference between base and tool
        int32_t tool_diff = dw_base.GetNumber() - probe_pos;
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
        // Move Z axis to the point before probing at speed 500 mm/min
        grbl_comm.JogInMachineCoodinates(GrblComm::AXIS_Z, z_position, 50000u);
        // Clear cmd if
        cmd_id = 0u;
        // Clear state
        state = PROBE_CNT;
      }
    }
  }

  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result ToolOffsetTab::ProcessCallback(const void* ptr)
{
  // We can start probing only in IDLE state and if probing isn't started yet
  if((grbl_comm.GetState() == GrblComm::IDLE) && (state == PROBE_CNT))
  {
    // Check buttons
    if(ptr == &get_offset_btn)
    {
      state = PROBE_TOOL;
      // Get current Z position in machine coordinates to return Z axis back after probing
      z_position = grbl_comm.GetAxisMachinePosition(GrblComm::AXIS_Z);
      // Try to probe Z axis -1 meter at speed 100 mm/min
      grbl_comm.ProbeAxisTowardWorkpiece(GrblComm::AXIS_Z, grbl_comm.GetAxisPosition(GrblComm::AXIS_Z) - 1000000, 100u, cmd_id);
    }
    else if(ptr == &get_base_btn)
    {
      state = PROBE_BASE;
      // Get current Z position in machine coordinates to return Z axis back after probing
      z_position = grbl_comm.GetAxisMachinePosition(GrblComm::AXIS_Z);
      // Try to probe Z axis -1 meter at speed 100 mm/min
      grbl_comm.ProbeAxisTowardWorkpiece(GrblComm::AXIS_Z, grbl_comm.GetAxisPosition(GrblComm::AXIS_Z) - 1000000, 100u, cmd_id);
    }
    else
    {
      ; // Do nothing - MISRA rule
    }
  }

  // Always good
  return Result::RESULT_OK;
}

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

  // Find Center button
  find_center_btn.SetParams("FIND CENTER", BORDER_W, dw_real[0].GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2, Font_8x12::GetInstance().GetCharH() * 5, true);
  find_center_btn.SetCallback(AppTask::GetCurrent());

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result CenterFinderTab::Show()
{
  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].Show(100);
    dw_real_name[i].Show(100);
  }

  // Find Center button
  find_center_btn.Show(102);

  // Request offsets to show it
  grbl_comm.RequestOffsets();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result CenterFinderTab::Hide()
{
  // Axis data
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].Hide();
    dw_real_name[i].Hide();
  }

  // Find Center button
  find_center_btn.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result CenterFinderTab::TimerExpired(uint32_t interval)
{
  // Update numbers with current position and position difference
  for(uint32_t i = 0u; i < NumberOf(dw_real); i++)
  {
    dw_real[i].SetNumber(grbl_comm.GetAxisPosition(i));
  }

  // Error check - if state isn't IDLE or RUN, we should abort probing sequence
  if((grbl_comm.GetState() != GrblComm::IDLE) && (grbl_comm.GetState() != GrblComm::RUN))
  {
    // Clear state
    state = PROBE_CNT;
    // Clear CMD ID
    cmd_id = 0u;
  }

  // If we send command and this command executed successfully
  if(cmd_id && (grbl_comm.GetCmdResult(cmd_id) == GrblComm::Status_OK))
  {
    // Check if command done executing
    if(grbl_comm.GetState() == GrblComm::IDLE)
    {
      // Iterations
      if(state == PROBE_START)
      {
        // We need absolute mode, otherwise we can screw up probe
        grbl_comm.SetAbsoluteMode();
        // Get current X position to return probe back after probing
        x_safe_pos = grbl_comm.GetAxisPosition(GrblComm::AXIS_X);
        // Try to probe X axis -1 meter at speed 200 mm/min
        grbl_comm.ProbeAxisTowardWorkpiece(GrblComm::AXIS_X, grbl_comm.GetAxisPosition(GrblComm::AXIS_X) - 1000000, 200u, cmd_id);
        // We start by measuring X min
        state = PROBE_FAST_X_MIN;
      }
      else if(state == PROBE_FAST_X_MIN)
      {
        // Get probe X position
        x_min_pos = grbl_comm.GetProbePosition(GrblComm::AXIS_X);
        // Move away from workpiece until probe contact lost
        grbl_comm.ProbeAxisAwayFromWorkpiece(GrblComm::AXIS_X, x_safe_pos, 200u, cmd_id);
        // Return after X min probing
        state = PROBE_FAST_X_MIN_RET;
      }
      if(state == PROBE_FAST_X_MIN_RET)
      {
        // Try to probe X axis at measured position - 1 mm at speed 50 mm/min
        grbl_comm.ProbeAxisTowardWorkpiece(GrblComm::AXIS_X, x_min_pos - 1000, 50u, cmd_id);
        // We start by measuring X min
        state = PROBE_X_MIN;
      }
      else if(state == PROBE_X_MIN)
      {
        // Get probe X position
        x_min_pos = grbl_comm.GetProbePosition(GrblComm::AXIS_X);
        // Rapid move X axis to the point before probing
        grbl_comm.MoveAxis(GrblComm::AXIS_X, x_safe_pos, 0u, cmd_id);
        // Return after X min probing
        state = PROBE_X_MIN_RET;
      }
      else if(state == PROBE_X_MIN_RET)
      {
        // Try to probe X axis +1 meter at speed 50 mm/min
        grbl_comm.ProbeAxisTowardWorkpiece(GrblComm::AXIS_X, grbl_comm.GetAxisPosition(GrblComm::AXIS_X) + 1000000, 50u, cmd_id);
        // Probe X max state
        state = PROBE_X_MAX;
      }
      else if(state == PROBE_X_MAX)
      {
        // Calculate X center position: X min + half from X min and X max difference
        x_safe_pos = x_min_pos + (grbl_comm.GetProbePosition(GrblComm::AXIS_X) - x_min_pos) / 2;
        // Rapid move X axis to the center
        grbl_comm.MoveAxis(GrblComm::AXIS_X, x_safe_pos, 0u, cmd_id);
        // Return after X max probing
        state = PROBE_X_MAX_RET;
      }
      else if(state == PROBE_X_MAX_RET)
      {
        // Get current Y position to return probe back after probing
        y_safe_pos = grbl_comm.GetAxisPosition(GrblComm::AXIS_Y);
        // Try to probe Y axis -1 meter at speed 50 mm/min
        grbl_comm.ProbeAxisTowardWorkpiece(GrblComm::AXIS_Y, grbl_comm.GetAxisPosition(GrblComm::AXIS_Y) - 1000000, 50u, cmd_id);
        // Probe Y min state
        state = PROBE_Y_MIN;
      }
      else if(state == PROBE_Y_MIN)
      {
        // Get probe X position
        y_min_pos = grbl_comm.GetProbePosition(GrblComm::AXIS_Y);
        // Rapid move X axis to the point before probing
        grbl_comm.MoveAxis(GrblComm::AXIS_Y, y_safe_pos, 0u, cmd_id);
        // Return after Y min probing
        state = PROBE_Y_MIN_RET;
      }
      else if(state == PROBE_Y_MIN_RET)
      {
        // Try to probe Y axis +1 meter at speed 50 mm/min
        grbl_comm.ProbeAxisTowardWorkpiece(GrblComm::AXIS_Y, grbl_comm.GetAxisPosition(GrblComm::AXIS_Y) + 1000000, 50u, cmd_id);
        // Probe Y max state
        state = PROBE_Y_MAX;
      }
      else if(state == PROBE_Y_MAX)
      {
        // Calculate Y center position: Y min + half from Y min and Y max difference
        y_safe_pos = y_min_pos + (grbl_comm.GetProbePosition(GrblComm::AXIS_Y) - y_min_pos) / 2;
        // Rapid move X axis to the center
        grbl_comm.MoveAxis(GrblComm::AXIS_Y, y_safe_pos, 0u, cmd_id);
        // We done probing
        state = PROBE_CNT;
        // Clear CMD ID
        cmd_id = 0u;
      }
      else
      {
        ; // Do nothing - MISRA rule
      }
    }
  }
  // Error check - if command was sent, but not accepted by controller TODO: do we need this?
  else if(cmd_id && (grbl_comm.GetCmdResult(cmd_id) != GrblComm::Status_Cmd_Not_Executed_Yet))
  {
    // Send Stop command
    grbl_comm.Stop();
    // Clear state
    state = PROBE_CNT;
    // Clear CMD ID
    cmd_id = 0u;
  }
  else
  {
    ; // Do nothing - MISRA rule
  }

  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result CenterFinderTab::ProcessCallback(const void* ptr)
{
  // We can start probing only in IDLE state and if probing isn't started yet
  if(grbl_comm.GetState() == GrblComm::IDLE)
  {
    // Check button
    if(ptr == &find_center_btn)
    {
      // Start probing
      state = PROBE_START;
    }
    else
    {
      ; // Do nothing - MISRA rule
    }
  }

  // Always good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProbeLineSequence function   ******************************************
// *****************************************************************************
Result CenterFinderTab::ProbeLineSequence(probe_state_t& state, uint8_t axis, int32_t dir, int32_t& safe_pos, int32_t& measured_pos)
{
  // Iterations
  if(state == PROBE_LINE_START)
  {
    // We need absolute mode, otherwise we can screw up probe
    grbl_comm.SetAbsoluteMode();
    // Get current axis position to return probe back after probing
    safe_pos = grbl_comm.GetAxisPosition(axis);
    // Try to probe axis +/- 1 meter at speed 200 mm/min
    grbl_comm.ProbeAxisTowardWorkpiece(axis, grbl_comm.GetAxisPosition(GrblComm::AXIS_X) + (1000000 * dir), 200u, cmd_id);
    // We start by measuring axis min
    state = PROBE_LINE_FAST;
  }
  else if(state == PROBE_LINE_FAST)
  {
    // Get probe axis position
    measured_pos = grbl_comm.GetProbePosition(axis);
    // Move away from workpiece until probe contact lost
    grbl_comm.ProbeAxisAwayFromWorkpiece(axis, safe_pos, 200u, cmd_id);
    // Return after axis min probing
    state = PROBE_LINE_FAST_RET;
  }
  if(state == PROBE_LINE_FAST_RET)
  {
    // Try to probe axis at measured position +/- 1 mm at speed 50 mm/min
    grbl_comm.ProbeAxisTowardWorkpiece(axis, measured_pos + (1000 * dir), 50u, cmd_id);
    // We start by measuring axis min
    state = PROBE_LINE_SLOW;
  }
  else if(state == PROBE_LINE_SLOW)
  {
    // Get probe axis position
    measured_pos = grbl_comm.GetProbePosition(axis);
    // Rapid move axis to the point before probing
    grbl_comm.MoveAxis(axis, safe_pos, 0u, cmd_id);
    // Return after axis min probing
    state = PROBE_LINE_SLOW_RET;
  }
  else if(state == PROBE_LINE_SLOW_RET)
  {
    // Probe max state
    state = PROBE_LINE_CNT;
  }
}
