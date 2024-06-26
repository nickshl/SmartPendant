//******************************************************************************
//  @file MillOpsScr.cpp
//  @author Nicolai Shlapunov
//
//  @details MillOpsScr: User MillOpsScr Class, implementation
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
#include "MillOpsScr.h"

#include "Application.h"

#include <stdarg.h>

// *****************************************************************************
// ***   PrintStr   ************************************************************
// *****************************************************************************
static void PrintStr(char*& buf, int32_t& len, const char* format, ...)
{
  // Argument list
  va_list arglist;
  // Create string
  va_start(arglist, format);
  vsnprintf(buf, len, format, arglist);
  va_end(arglist);
  // Find string length, increase pointer and decrease remaining size
  size_t n = strlen(buf);
  buf += n;
  len -= n;
  // Add \r\n to the end
  if(len > 2)
  {
    buf[0] = '\r';
    buf[1] = '\n';
    buf[2] = '\0';
    buf += 2;
    len -= 2;
  }
}

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
MillOpsScr& MillOpsScr::GetInstance()
{
  static MillOpsScr MillOpsScr;
  return MillOpsScr;
}

// *****************************************************************************
// ***   MillOpsScr Setup   ****************************************************
// *****************************************************************************
Result MillOpsScr::Setup(int32_t y, int32_t height)
{
  // Tabs for screens(first call to set parameters)
  tabs.SetParams(0, y, display_drv.GetScreenW(), 40, Tabs::MAX_TABS);
  // Screens & Captions
  tabs_cnt = 0u; // Clear tabs since Setup() can be called multiple times
//  // Fill tabs
//  tabs.SetText(tabs_cnt, "Drill", nullptr, Font_10x18::GetInstance());
//  tab[tabs_cnt++] = &DrillTab::GetInstance();
  // Fill tabs
  tabs.SetText(tabs_cnt, "Drill", nullptr, Font_10x18::GetInstance());
  tab[tabs_cnt++] = &DrillGeneratorTab::GetInstance();
  // Fill tabs
  tabs.SetText(tabs_cnt, "Enlarge", nullptr, Font_10x18::GetInstance());
  tab[tabs_cnt++] = &EnlargeGeneratorTab::GetInstance();
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
Result MillOpsScr::Show()
{
  // Show tabs
  tabs.Show(2000);

  // Stop button
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
Result MillOpsScr::Hide()
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
Result MillOpsScr::TimerExpired(uint32_t interval)
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
Result MillOpsScr::ProcessCallback(const void* ptr)
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
// ***   Private: ProcessButtonCallback function   *****************************
// *****************************************************************************
Result MillOpsScr::ProcessButtonCallback(MillOpsScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    MillOpsScr& ths = *obj_ptr;
    // Get pressed button
    InputDrv::ButtonCallbackData btn = *((InputDrv::ButtonCallbackData*)ptr);

    // Buttons to switch tabs
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
void MillOpsScr::ChangeTab(uint8_t tabn)
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
MillOpsScr::MillOpsScr() : right_btn(Application::GetInstance().GetRightButton()) {};

// ******************************************************************************
// ******************************************************************************
// ***   DRILL GENERATOR TAB   **************************************************
// ******************************************************************************
// ******************************************************************************

// ******************************************************************************
// ***   Get Instance   *********************************************************
// ******************************************************************************
DrillGeneratorTab& DrillGeneratorTab::GetInstance()
{
  static DrillGeneratorTab drill_generator_tab;
  return drill_generator_tab;
}

// ******************************************************************************
// ***   DrillGeneratorTab Setup   **********************************************
// ******************************************************************************
Result DrillGeneratorTab::Setup(int32_t y, int32_t height)
{
  int32_t start_y = y + BORDER_W;

  // Data window height
  uint32_t window_height = 57u;

  // Drill distance
  dw_drill_distance.SetParams(BORDER_W, start_y, display_drv.GetScreenW() - BORDER_W*2,  window_height, 15u, grbl_comm.GetUnitsPrecision());
  dw_drill_distance.SetBorder(BORDER_W, COLOR_RED);
  dw_drill_distance.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_drill_distance.SetNumber(0);
  dw_drill_distance.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  dw_drill_distance.SetCallback(AppTask::GetCurrent());
  dw_drill_distance.SetActive(true);
  // Drill distance caption
  dw_drill_distance_name.SetParams("DISTANCE", dw_drill_distance.GetStartX() + BORDER_W*2, dw_drill_distance.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // Drill stepover
  dw_drill_stepover.SetParams(BORDER_W, dw_drill_distance.GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2,  window_height, 15u, grbl_comm.GetUnitsPrecision());
  dw_drill_stepover.SetBorder(BORDER_W, COLOR_RED);
  dw_drill_stepover.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_drill_stepover.SetNumber(0);
  dw_drill_stepover.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  dw_drill_stepover.SetCallback(AppTask::GetCurrent());
  dw_drill_stepover.SetActive(true);
  // Drill stepover caption
  dw_drill_stepover_name.SetParams("STEPOVER", dw_drill_stepover.GetStartX() + BORDER_W*2, dw_drill_stepover.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // Drill clearance
  dw_drill_clearance.SetParams(BORDER_W, dw_drill_stepover.GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2,  window_height, 15u, grbl_comm.GetUnitsPrecision());
  dw_drill_clearance.SetBorder(BORDER_W, COLOR_RED);
  dw_drill_clearance.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_drill_clearance.SetNumber(0);
  dw_drill_clearance.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  dw_drill_clearance.SetCallback(AppTask::GetCurrent());
  dw_drill_clearance.SetActive(true);
  // Drill clearance caption
  dw_drill_clearance_name.SetParams("CLEARANCE", dw_drill_clearance.GetStartX() + BORDER_W*2, dw_drill_clearance.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].SetParams(grbl_comm.IsMetric() ? scale_str_metric[i] : scale_str_imperial[i], i*(display_drv.GetScreenW() / 3) + BORDER_W, dw_drill_clearance.GetEndY() + BORDER_W*2, display_drv.GetScreenW() / 3 - BORDER_W*2, window_height, true);
    scale_btn[i].SetCallback(AppTask::GetCurrent());
    scale_btn[i].SetSpacing(3u);
  }

  // Drill speed
  dw_drill_speed.SetParams(BORDER_W, scale_btn[0].GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2,  window_height, 15u, 0u);
  dw_drill_speed.SetBorder(BORDER_W, COLOR_RED);
  dw_drill_speed.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_drill_speed.SetNumber(0);
  dw_drill_speed.SetUnits(grbl_comm.IsMetric() ? "mm/min" : "inches/min", DataWindow::RIGHT);
  dw_drill_speed.SetCallback(AppTask::GetCurrent());
  dw_drill_speed.SetActive(true);
  // Drill speed caption
  dw_drill_speed_name.SetParams("SPEED", dw_drill_speed.GetStartX() + BORDER_W*2, dw_drill_speed.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result DrillGeneratorTab::Show()
{
  // Data windows
  dw_drill_distance.Show(100);
  dw_drill_stepover.Show(100);
  dw_drill_clearance.Show(100);
  dw_drill_speed.Show(100);
  // String for caption
  dw_drill_distance_name.Show(101);
  dw_drill_stepover_name.Show(101);
  dw_drill_clearance_name.Show(101);
  dw_drill_speed_name.Show(101);

  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Show(101);
  }

  // Drill button
  left_btn.Show(102);

  // Set encoder callback handler
  InputDrv::GetInstance().AddEncoderCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessEncoderCallback), this, enc_cble);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result DrillGeneratorTab::Hide()
{
  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);

  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Hide();
  }

  // String for caption
  dw_drill_distance_name.Hide();
  dw_drill_stepover_name.Hide();
  dw_drill_clearance_name.Hide();
  dw_drill_speed_name.Hide();
  // Data windows
  dw_drill_distance.Hide();
  dw_drill_stepover.Hide();
  dw_drill_clearance.Hide();
  dw_drill_speed.Hide();

  // Drill button
  left_btn.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result DrillGeneratorTab::TimerExpired(uint32_t interval)
{
  // Left soft button text
  Application::GetInstance().UpdateLeftButtonIdleText("Generate");

  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result DrillGeneratorTab::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  // Process Run button. Since we can call this handler after press of physical
  // button, we have to check if Run button is active.
  if(ptr == &left_btn)
  {
    // If we already run program
    // We can start probing only in IDLE state and if probing isn't started yet
    if(grbl_comm.GetState() != GrblComm::IDLE)
    {
      result = Result::ERR_UNHANDLED_REQUEST; // For Application to handle it
    }
    else
    {
      // Generate GCode for ProgramSender
      GenerateGcode();
      // Switch to the Program Sender screen
      Application::GetInstance().ChangeScreen(ProgramSender::GetInstance());
    }
  }
  else if(ptr == &dw_drill_distance)
  {
    dw_drill_distance.SetSeleced(true);
    dw_drill_stepover.SetSeleced(false);
    dw_drill_clearance.SetSeleced(false);
    dw_drill_speed.SetSeleced(false);
  }
  else if(ptr == &dw_drill_stepover)
  {
    dw_drill_distance.SetSeleced(false);
    dw_drill_stepover.SetSeleced(true);
    dw_drill_clearance.SetSeleced(false);
    dw_drill_speed.SetSeleced(false);
  }
  else if(ptr == &dw_drill_clearance)
  {
    dw_drill_distance.SetSeleced(false);
    dw_drill_stepover.SetSeleced(false);
    dw_drill_clearance.SetSeleced(true);
    dw_drill_speed.SetSeleced(false);
  }
  else if(ptr == &dw_drill_speed)
  {
    dw_drill_distance.SetSeleced(false);
    dw_drill_stepover.SetSeleced(false);
    dw_drill_clearance.SetSeleced(false);
    dw_drill_speed.SetSeleced(true);
  }
  else if(ptr == &scale_btn[0])
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
  else
  {
    ; // Do nothing - MISRA rule
  }

  // Set pressed state for selected scale button and unpressed for unselected ones
  scale_btn[0u].SetPressed(scale == 1);
  scale_btn[1u].SetPressed(scale == 10);
  scale_btn[2u].SetPressed(scale == 100);

  // Return result
  return result;
}

// ******************************************************************************
// ***   Private: ProcessEncoderCallback function   *****************************
// ******************************************************************************
Result DrillGeneratorTab::ProcessEncoderCallback(DrillGeneratorTab* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    DrillGeneratorTab& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    if(ths.dw_drill_distance.IsSeleced())
    {
      ths.dw_drill_distance.SetNumber(ths.dw_drill_distance.GetNumber() + enc_val * ths.scale);
    }
    else if(ths.dw_drill_stepover.IsSeleced())
    {
      ths.dw_drill_stepover.SetNumber(ths.dw_drill_stepover.GetNumber() + enc_val * ths.scale);
    }
    else if(ths.dw_drill_clearance.IsSeleced())
    {
      ths.dw_drill_clearance.SetNumber(ths.dw_drill_clearance.GetNumber() + enc_val * ths.scale);
    }
    else if(ths.dw_drill_speed.IsSeleced())
    {
      // Calculate new speed
      int32_t new_number = ths.dw_drill_speed.GetNumber() + enc_val;
      // Speed can't be negative(and zero too, but whatever)
      if(new_number < 0) new_number = 0;
      // Set new speed number
      ths.dw_drill_speed.SetNumber(new_number);
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

// ******************************************************************************
// ***   Private: GenerateGcode function   **************************************
// ******************************************************************************
Result DrillGeneratorTab::GenerateGcode()
{
  int32_t size = 8192;
  char *txt = ProgramSender::GetInstance().AllocateDataBuffer(size);

  if(txt != nullptr)
  {
    // Drilling parameters
    int32_t drill_distance = dw_drill_distance.GetNumber();
    int32_t drill_stepover = dw_drill_stepover.GetNumber();
    int32_t drill_speed = dw_drill_speed.GetNumber();
    // Clear drill progress
    int32_t drill_progress = 0;
    int32_t clearance = 0;

    // If stepover set to zero
    if(drill_stepover == 0)
    {
      // Drill all way in one go
      drill_stepover = drill_distance;
    }

    PrintStr(txt, size, "G91; Relative mode");

    while(drill_progress != drill_distance)
    {
      // Do not move drill down first iteration
      if(drill_progress != 0)
      {
        // Start apply clearance after first iteration
        clearance = dw_drill_clearance.GetNumber();
        // Calculate position
        int32_t pos = drill_progress - clearance;
        PrintStr(txt, size, "G0 Z%li.%03i", -pos/1000, abs(pos%1000));
      }
      // Update progress
      drill_progress += drill_stepover;
      // Check if it more than we need and correct it
      if(drill_progress > drill_distance) drill_progress = drill_distance;
      // Drill piece
      PrintStr(txt, size, "G1 Z%li.%03i F%li", -(drill_stepover + clearance)/1000, abs((drill_stepover + clearance) % 1000), drill_speed);
      // Rapid move out to clear chips
      PrintStr(txt, size, "G0 Z%li.%03i", drill_progress/1000, abs(drill_progress % 1000));
    }
  }

  return Result::RESULT_OK;
}

// ******************************************************************************
// ***   Private constructor   **************************************************
// ******************************************************************************
DrillGeneratorTab::DrillGeneratorTab() : left_btn(Application::GetInstance().GetLeftButton()) {};

// ******************************************************************************
// ******************************************************************************
// ***   ENLARGE GENERATOR TAB   ************************************************
// ******************************************************************************
// ******************************************************************************

// ******************************************************************************
// ***   Get Instance   *********************************************************
// ******************************************************************************
EnlargeGeneratorTab& EnlargeGeneratorTab::GetInstance()
{
  static EnlargeGeneratorTab drill_generator_tab;
  return drill_generator_tab;
}

// ******************************************************************************
// ***   EnlargeGeneratorTab Setup   **********************************************
// ******************************************************************************
Result EnlargeGeneratorTab::Setup(int32_t y, int32_t height)
{
  int32_t start_y = y + BORDER_W;

  // Data window height
  uint32_t window_height = 57u;

  // Drill distance
  dw_hole_diameter.SetParams(BORDER_W, start_y, display_drv.GetScreenW() - BORDER_W*2,  window_height, 15u, grbl_comm.GetUnitsPrecision());
  dw_hole_diameter.SetBorder(BORDER_W, COLOR_RED);
  dw_hole_diameter.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_hole_diameter.SetNumber(0);
  dw_hole_diameter.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  dw_hole_diameter.SetCallback(AppTask::GetCurrent());
  dw_hole_diameter.SetActive(true);
  // Drill distance caption
  dw_hole_diameter_name.SetParams("DIAMETER", dw_hole_diameter.GetStartX() + BORDER_W*2, dw_hole_diameter.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // Drill stepover
  dw_stepover.SetParams(BORDER_W, dw_hole_diameter.GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2,  window_height, 15u, grbl_comm.GetUnitsPrecision());
  dw_stepover.SetBorder(BORDER_W, COLOR_RED);
  dw_stepover.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_stepover.SetNumber(0);
  dw_stepover.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  dw_stepover.SetCallback(AppTask::GetCurrent());
  dw_stepover.SetActive(true);
  // Drill stepover caption
  dw_stepover_name.SetParams("STEPOVER", dw_stepover.GetStartX() + BORDER_W*2, dw_stepover.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // Drill clearance
  dw_endmill_diameter.SetParams(BORDER_W, dw_stepover.GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2,  window_height, 15u, grbl_comm.GetUnitsPrecision());
  dw_endmill_diameter.SetBorder(BORDER_W, COLOR_RED);
  dw_endmill_diameter.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_endmill_diameter.SetNumber(0);
  dw_endmill_diameter.SetUnits(grbl_comm.GetReportUnits(), DataWindow::RIGHT);
  dw_endmill_diameter.SetCallback(AppTask::GetCurrent());
  dw_endmill_diameter.SetActive(true);
  // Drill clearance caption
  dw_endmill_diameter_name.SetParams("ENDMILL", dw_endmill_diameter.GetStartX() + BORDER_W*2, dw_endmill_diameter.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].SetParams(grbl_comm.IsMetric() ? scale_str_metric[i] : scale_str_imperial[i], i*(display_drv.GetScreenW() / 3) + BORDER_W, dw_endmill_diameter.GetEndY() + BORDER_W*2, display_drv.GetScreenW() / 3 - BORDER_W*2, window_height, true);
    scale_btn[i].SetCallback(AppTask::GetCurrent());
    scale_btn[i].SetSpacing(3u);
  }

  // Drill speed
  dw_speed.SetParams(BORDER_W, scale_btn[0].GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2,  window_height, 15u, 0u);
  dw_speed.SetBorder(BORDER_W, COLOR_RED);
  dw_speed.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_speed.SetNumber(0);
  dw_speed.SetUnits(grbl_comm.IsMetric() ? "mm/min" : "inches/min", DataWindow::RIGHT);
  dw_speed.SetCallback(AppTask::GetCurrent());
  dw_speed.SetActive(true);
  // Drill speed caption
  dw_speed_name.SetParams("SPEED", dw_speed.GetStartX() + BORDER_W*2, dw_speed.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result EnlargeGeneratorTab::Show()
{
  // Data windows
  dw_hole_diameter.Show(100);
  dw_stepover.Show(100);
  dw_endmill_diameter.Show(100);
  dw_speed.Show(100);
  // String for caption
  dw_hole_diameter_name.Show(101);
  dw_stepover_name.Show(101);
  dw_endmill_diameter_name.Show(101);
  dw_speed_name.Show(101);

  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Show(101);
  }

  // Drill button
  left_btn.Show(102);

  // Set encoder callback handler
  InputDrv::GetInstance().AddEncoderCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessEncoderCallback), this, enc_cble);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result EnlargeGeneratorTab::Hide()
{
  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);

  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Hide();
  }

  // String for caption
  dw_hole_diameter_name.Hide();
  dw_stepover_name.Hide();
  dw_endmill_diameter_name.Hide();
  dw_speed_name.Hide();
  // Data windows
  dw_hole_diameter.Hide();
  dw_stepover.Hide();
  dw_endmill_diameter.Hide();
  dw_speed.Hide();

  // Drill button
  left_btn.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result EnlargeGeneratorTab::TimerExpired(uint32_t interval)
{
  // Left soft button text
  Application::GetInstance().UpdateLeftButtonIdleText("Generate");

  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result EnlargeGeneratorTab::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  // Process Run button. Since we can call this handler after press of physical
  // button, we have to check if Run button is active.
  if(ptr == &left_btn)
  {
    // If we already run program
    // We can start probing only in IDLE state and if probing isn't started yet
    if((grbl_comm.GetState() != GrblComm::IDLE) && (grbl_comm.GetState() != GrblComm::UNKNOWN))
    {
      result = Result::ERR_UNHANDLED_REQUEST; // For Application to handle it
    }
    else
    {
      // Generate GCode for ProgramSender
      GenerateGcode();
      // Switch to the Program Sender screen
      Application::GetInstance().ChangeScreen(ProgramSender::GetInstance());
    }
  }
  else if(ptr == &dw_hole_diameter)
  {
    dw_hole_diameter.SetSeleced(true);
    dw_stepover.SetSeleced(false);
    dw_endmill_diameter.SetSeleced(false);
    dw_speed.SetSeleced(false);
  }
  else if(ptr == &dw_stepover)
  {
    dw_hole_diameter.SetSeleced(false);
    dw_stepover.SetSeleced(true);
    dw_endmill_diameter.SetSeleced(false);
    dw_speed.SetSeleced(false);
  }
  else if(ptr == &dw_endmill_diameter)
  {
    dw_hole_diameter.SetSeleced(false);
    dw_stepover.SetSeleced(false);
    dw_endmill_diameter.SetSeleced(true);
    dw_speed.SetSeleced(false);
  }
  else if(ptr == &dw_speed)
  {
    dw_hole_diameter.SetSeleced(false);
    dw_stepover.SetSeleced(false);
    dw_endmill_diameter.SetSeleced(false);
    dw_speed.SetSeleced(true);
  }
  else if(ptr == &scale_btn[0])
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
  else
  {
    ; // Do nothing - MISRA rule
  }

  // Set pressed state for selected scale button and unpressed for unselected ones
  scale_btn[0u].SetPressed(scale == 1);
  scale_btn[1u].SetPressed(scale == 10);
  scale_btn[2u].SetPressed(scale == 100);

  // Return result
  return result;
}

// ******************************************************************************
// ***   Private: ProcessEncoderCallback function   *****************************
// ******************************************************************************
Result EnlargeGeneratorTab::ProcessEncoderCallback(EnlargeGeneratorTab* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    EnlargeGeneratorTab& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    if(ths.dw_hole_diameter.IsSeleced())
    {
      ths.dw_hole_diameter.SetNumber(ths.dw_hole_diameter.GetNumber() + enc_val * ths.scale);
    }
    else if(ths.dw_stepover.IsSeleced())
    {
      ths.dw_stepover.SetNumber(ths.dw_stepover.GetNumber() + enc_val * ths.scale);
    }
    else if(ths.dw_endmill_diameter.IsSeleced())
    {
      ths.dw_endmill_diameter.SetNumber(ths.dw_endmill_diameter.GetNumber() + enc_val * ths.scale);
    }
    else if(ths.dw_speed.IsSeleced())
    {
      // Calculate new speed
      int32_t new_number = ths.dw_speed.GetNumber() + enc_val;
      // Speed can't be negative(and zero too, but whatever)
      if(new_number < 0) new_number = 0;
      // Set new speed number
      ths.dw_speed.SetNumber(new_number);
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

// ******************************************************************************
// ***   Private: GenerateGcode function   **************************************
// ******************************************************************************
Result EnlargeGeneratorTab::GenerateGcode()
{
  int32_t size = 8192;
  char *txt = ProgramSender::GetInstance().AllocateDataBuffer(size);

  if(txt != nullptr)
  {
    // Drilling parameters
    int32_t endmill_radius = dw_endmill_diameter.GetNumber() / 2;
    int32_t hole_radius = (dw_hole_diameter.GetNumber() / 2) - endmill_radius;
    int32_t stepover = dw_stepover.GetNumber();
    int32_t speed = dw_speed.GetNumber();
    // Clear drill progress
    int32_t enlarge_progress = 0;
    int32_t center = 0;

    // Switch to relative mode
    PrintStr(txt, size, "G91; Relative mode");

    // Move half stepover to center
    PrintStr(txt, size, "G1 X-%li.%03i F%li", (stepover / 2) / 1000, abs((stepover / 2) % 1000), speed);

    // Find number of iterations: radius divided by stepover
    uint32_t n = hole_radius / stepover;
    // We can lost last operation if last step isn't equal to stepover
    n += (hole_radius % stepover) ? 1u : 0u;
    // Number of steps should be odd to make round hole. If it already odd, hole won't be round since even step isn't have center in center.
    n += (n%2) ? 1u : 2u;

    // Generate cycle
    for(uint32_t i = 0u; i < n; i++)
    {
      // Increase diameter
      enlarge_progress += stepover;
      // Check result and correct it if it needed
      if(enlarge_progress > hole_radius)
      {
        enlarge_progress = hole_radius;
      }
      // Find new I value
      center = enlarge_progress / 2;
      // Every other iteration minuses should be added
      if(i%2)
      {
        PrintStr(txt, size, "G3 I-%li.%03i X-%li.%03i F%li", center/1000, abs(center%1000), enlarge_progress/1000, abs(enlarge_progress%1000), (speed * enlarge_progress) / (enlarge_progress + endmill_radius));
      }
      else
      {
        PrintStr(txt, size, "G3 I%li.%03i X%li.%03i F%li", center/1000, abs(center%1000), enlarge_progress/1000, abs(enlarge_progress%1000), (speed * enlarge_progress) / (enlarge_progress + endmill_radius));
      }
    }

    // Return to center. n always have to be odd.
    PrintStr(txt, size, "G0 X%li.%03i", center/1000, abs(center%1000));
  }

  return Result::RESULT_OK;
}

// ******************************************************************************
// ***   Private constructor   **************************************************
// ******************************************************************************
EnlargeGeneratorTab::EnlargeGeneratorTab() : left_btn(Application::GetInstance().GetLeftButton()) {};
