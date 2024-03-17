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
// ***   ProbeScr Setup   ***********************************************
// *****************************************************************************
Result ProbeScr::Setup(int32_t y, int32_t height)
{
  int32_t start_y = y + BORDER_W;
  // Data window height
  uint32_t window_height = Font_8x12::GetInstance().GetCharH() * 5u;

  // Tool offset Name
  name_tool.SetParams("TOOL OFFSET", 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
//  name_tool.SetScale(2u);
  name_tool.Move((display_drv.GetScreenW()  / 2) - (name_tool.GetWidth() / 2), start_y + BORDER_W);
  // Tool offset position
  dw_tool.SetParams(BORDER_W, name_tool.GetEndY() + BORDER_W, display_drv.GetScreenW() - BORDER_W*2,  window_height, 4u, 3u);
  dw_tool.SetBorder(BORDER_W, COLOR_RED);
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
//  name_base.SetScale(2u);
  name_base.Move((display_drv.GetScreenW()  / 2) - (name_base.GetWidth() / 2), get_offset_btn.GetEndY() + BORDER_W * 2u);
  // Tool offset position
  dw_base.SetParams(BORDER_W, name_base.GetEndY() + BORDER_W, display_drv.GetScreenW() - BORDER_W*2,  window_height, 4u, 3u);
  dw_base.SetBorder(BORDER_W, COLOR_RED);
  dw_base.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_base.SetNumber(0); // Get current position at startup
  dw_base.SetUnits("mm", DataWindow::RIGHT);
  dw_base.SetCallback(AppTask::GetCurrent());
  dw_base.SetActive(false);
  // Measure offset button
  get_base_btn.SetParams("MEASURE BASE", BORDER_W, dw_base.GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2, Font_8x12::GetInstance().GetCharH() * 5, true);
  get_base_btn.SetCallback(AppTask::GetCurrent());

  // Stop button
  right_btn.SetParams("Stop", display_drv.GetScreenW() - display_drv.GetScreenW() / 2 + BORDER_W, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3, display_drv.GetScreenW() / 2 - BORDER_W, Font_8x12::GetInstance().GetCharH() * 3, true);
  right_btn.SetCallback(AppTask::GetCurrent());

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
Result ProbeScr::Show()
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

  // Stop button
  right_btn.Show(102);

  // Set encoder callback handler
  InputDrv::GetInstance().AddEncoderCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessEncoderCallback), this, enc_cble);
  // Set callback handler for left and right buttons
  InputDrv::GetInstance().AddButtonsCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this, InputDrv::BTNM_LEFT | InputDrv::BTNM_RIGHT, btn_cble);

  // Request offsets to show it
  grbl_comm.RequestOffsets();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result ProbeScr::Hide()
{
  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);
  // Delete buttons callback handler
  InputDrv::GetInstance().DeleteButtonsCallbackHandler(btn_cble);

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

  // Reset button
  right_btn.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result ProbeScr::TimerExpired(uint32_t interval)
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
      int32_t probe_pos = grbl_comm.GetProbePosition(GrblComm::AXIS_Z);
      // If we tied base - update base data window
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
Result ProbeScr::ProcessCallback(const void* ptr)
{
  if(ptr == &right_btn)
  {
    // Send Stop command
    grbl_comm.Stop();
  }

  // We can start probing only in IDLE state and if probing isn't started yet
  if((grbl_comm.GetState() == GrblComm::IDLE) && (state == PROBE_CNT))
  {
    // Check buttons
    if(ptr == &get_offset_btn)
    {
        state = PROBE_TOOL;
        // Get current Z position in machine coordinates to return Z axis back after probing
        z_position = grbl_comm.GetAxisAbsolutePosition(GrblComm::AXIS_Z);
        // Try to probe Z axis -1 meter
        grbl_comm.ProbeAxisTowardWorkpiece(GrblComm::AXIS_Z, grbl_comm.GetAxisPosition(GrblComm::AXIS_Z) - 1000000, cmd_id);
    }
    else if(ptr == &get_base_btn)
    {
        state = PROBE_BASE;
        // Get current Z position in machine coordinates to return Z axis back after probing
        z_position = grbl_comm.GetAxisAbsolutePosition(GrblComm::AXIS_Z);
        // Try to probe Z axis -1 meter
        grbl_comm.ProbeAxisTowardWorkpiece(GrblComm::AXIS_Z, grbl_comm.GetAxisPosition(GrblComm::AXIS_Z) - 1000000, cmd_id);
    }
    else
    {
      ; // Do nothing - MISRA rule
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
Result ProbeScr::ProcessEncoderCallback(ProbeScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ProbeScr& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    // Process it
    if(enc_val != 0)
    {
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
void ProbeScr::UpdateObjects(void)
{
}
