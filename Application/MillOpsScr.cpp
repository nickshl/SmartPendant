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
  // Set middle scale
  ProcessCallback(&scale_btn[1u]);

  // Drill feed
  dw_drill_feed.SetParams(BORDER_W, scale_btn[0].GetEndY() + BORDER_W*2, display_drv.GetScreenW() - BORDER_W*2,  window_height, 15u, 0u);
  dw_drill_feed.SetBorder(BORDER_W, COLOR_RED);
  dw_drill_feed.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_drill_feed.SetNumber(0);
  dw_drill_feed.SetUnits(grbl_comm.IsMetric() ? "mm/min" : "inches/min", DataWindow::RIGHT);
  dw_drill_feed.SetCallback(AppTask::GetCurrent());
  dw_drill_feed.SetActive(true);
  // Drill feed caption
  dw_drill_feed_name.SetParams("FEED", dw_drill_feed.GetStartX() + BORDER_W*2, dw_drill_feed.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_12x16::GetInstance());

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
  dw_drill_feed.Show(100);
  // String for caption
  dw_drill_distance_name.Show(101);
  dw_drill_stepover_name.Show(101);
  dw_drill_clearance_name.Show(101);
  dw_drill_feed_name.Show(101);

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
  dw_drill_feed_name.Hide();
  // Data windows
  dw_drill_distance.Hide();
  dw_drill_stepover.Hide();
  dw_drill_clearance.Hide();
  dw_drill_feed.Hide();

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
    dw_drill_distance.SetSelected(true);
    dw_drill_stepover.SetSelected(false);
    dw_drill_clearance.SetSelected(false);
    dw_drill_feed.SetSelected(false);
  }
  else if(ptr == &dw_drill_stepover)
  {
    dw_drill_distance.SetSelected(false);
    dw_drill_stepover.SetSelected(true);
    dw_drill_clearance.SetSelected(false);
    dw_drill_feed.SetSelected(false);
  }
  else if(ptr == &dw_drill_clearance)
  {
    dw_drill_distance.SetSelected(false);
    dw_drill_stepover.SetSelected(false);
    dw_drill_clearance.SetSelected(true);
    dw_drill_feed.SetSelected(false);
  }
  else if(ptr == &dw_drill_feed)
  {
    dw_drill_distance.SetSelected(false);
    dw_drill_stepover.SetSelected(false);
    dw_drill_clearance.SetSelected(false);
    dw_drill_feed.SetSelected(true);
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

    // Pointer to DataWindow which data should be changed
    DataWindow* pdw = nullptr;
    // Addition value - by default multiplied by scale to minimize code
    int32_t add_val = enc_val * ths.scale;

    // Find pointer for currently selected DataWindow
    if(ths.dw_drill_distance.IsSelected())
    {
      pdw = &ths.dw_drill_distance;
    }
    else if(ths.dw_drill_stepover.IsSelected())
    {
      pdw = &ths.dw_drill_stepover;
    }
    else if(ths.dw_drill_clearance.IsSelected())
    {
      pdw = &ths.dw_drill_clearance;
    }
    else if(ths.dw_drill_feed.IsSelected())
    {
      pdw = &ths.dw_drill_feed;
      add_val = enc_val; // Feed doesn't need scale to be used
    }
    else
    {
      ; // Do nothing - MISRA rule
    }

    // If we found currently selected DataWindow
    if(pdw != nullptr)
    {
      // Calculate new value
      int32_t new_number = pdw->GetNumber() + add_val;
      // Value can't be negative(and probably zero too, but whatever)
      if(new_number < 0) new_number = 0;
      // Set new feed number
      pdw->SetNumber(new_number);
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
  // Drilling parameters
  int32_t drill_distance = dw_drill_distance.GetNumber();
  int32_t drill_stepover = dw_drill_stepover.GetNumber();
  int32_t drill_feed = dw_drill_feed.GetNumber();

  // If stepover set to zero or or greater than drill distance
  if((drill_stepover == 0) || (drill_stepover >  drill_distance))
  {
    // Drill all way in one go
    drill_stepover = drill_distance;
  }

  // Calculate size (approx. Number of steps * 3 lines per step * 32 bytes per line)
  int32_t size = (drill_distance / drill_stepover + 1) * 3 * 32;
  // Allocate buffer
  char *txt = ProgramSender::GetInstance().AllocateDataBuffer(size);

  // If allocation is successful - generate G-code
  if(txt != nullptr)
  {
    // Drill progress and clearance - 0 by default for first step
    int32_t drill_progress = 0;
    int32_t clearance = 0;

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
      PrintStr(txt, size, "G1 Z%li.%03i F%li", -(drill_stepover + clearance)/1000, (drill_stepover + clearance) % 1000, drill_feed);
      // Rapid move out to clear chips
      PrintStr(txt, size, "G0 Z%li.%03i", drill_progress/1000, drill_progress % 1000);
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
  static EnlargeGeneratorTab enlarge_generator_tab;
  return enlarge_generator_tab;
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
  // Set middle scale
  ProcessCallback(&scale_btn[1u]);

  // Drill feed
  dw_feed.SetParams(BORDER_W, scale_btn[0].GetEndY() + BORDER_W*2, display_drv.GetScreenW()/2 - BORDER_W*2,  window_height, 6u, 0u);
  dw_feed.SetBorder(BORDER_W, COLOR_RED);
  dw_feed.SetDataFont(Font_8x12::GetInstance(), 2u);
  dw_feed.SetNumber(0);
  dw_feed.SetUnits(grbl_comm.IsMetric() ? "mm/min" : "inches/min", DataWindow::RIGHT);
  dw_feed.SetCallback(AppTask::GetCurrent());
  dw_feed.SetActive(true);
  // Drill feed caption
  dw_feed_name.SetParams("FEED", dw_feed.GetStartX() + BORDER_W*2, dw_feed.GetStartY() + BORDER_W*2, COLOR_WHITE, Font_8x12::GetInstance());

  // Direction button
  direction_btn.SetParams(direction_str[direction], display_drv.GetScreenW()/2 + BORDER_W, dw_feed.GetStartY(), dw_feed.GetWidth(), dw_feed.GetHeight(), true);
  direction_btn.SetFont(Font_12x16::GetInstance());
  direction_btn.SetCallback(AppTask::GetCurrent());

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
  dw_feed.Show(100);
  // String for caption
  dw_hole_diameter_name.Show(101);
  dw_stepover_name.Show(101);
  dw_endmill_diameter_name.Show(101);
  dw_feed_name.Show(101);

  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Show(101);
  }

  // Direction button
  direction_btn.Show(102);

  // Generate button
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
  dw_feed_name.Hide();
  // Data windows
  dw_hole_diameter.Hide();
  dw_stepover.Hide();
  dw_endmill_diameter.Hide();
  dw_feed.Hide();

  // Direction button
  direction_btn.Hide();

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
    dw_hole_diameter.SetSelected(true);
    dw_stepover.SetSelected(false);
    dw_endmill_diameter.SetSelected(false);
    dw_feed.SetSelected(false);
  }
  else if(ptr == &dw_stepover)
  {
    dw_hole_diameter.SetSelected(false);
    dw_stepover.SetSelected(true);
    dw_endmill_diameter.SetSelected(false);
    dw_feed.SetSelected(false);
  }
  else if(ptr == &dw_endmill_diameter)
  {
    dw_hole_diameter.SetSelected(false);
    dw_stepover.SetSelected(false);
    dw_endmill_diameter.SetSelected(true);
    dw_feed.SetSelected(false);
  }
  else if(ptr == &dw_feed)
  {
    dw_hole_diameter.SetSelected(false);
    dw_stepover.SetSelected(false);
    dw_endmill_diameter.SetSelected(false);
    dw_feed.SetSelected(true);
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
  else if(ptr == &direction_btn)
  {
    direction = !direction;
    direction_btn.SetString(direction_str[direction]);
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

    // Pointer to DataWindow which data should be changed
    DataWindow* pdw = nullptr;
    // Addition value - by default multiplied by scale to minimize code
    int32_t add_val = enc_val * ths.scale;

    // Find pointer for currently selected DataWindow
    if(ths.dw_hole_diameter.IsSelected())
    {
      pdw = &ths.dw_hole_diameter;
    }
    else if(ths.dw_stepover.IsSelected())
    {
      pdw = &ths.dw_stepover;
    }
    else if(ths.dw_endmill_diameter.IsSelected())
    {
      pdw = &ths.dw_endmill_diameter;
    }
    else if(ths.dw_feed.IsSelected())
    {
      pdw = &ths.dw_feed;
      add_val = enc_val; // Feed doesn't need scale to be used
    }
    else
    {
      ; // Do nothing - MISRA rule
    }

    // If we found currently selected DataWindow
    if(pdw != nullptr)
    {
      // Calculate new value
      int32_t new_number = pdw->GetNumber() + add_val;
      // Value can't be negative(and probably zero too, but whatever)
      if(new_number < 0) new_number = 0;
      // Set new feed number
      pdw->SetNumber(new_number);
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
  // Drilling parameters
  int32_t endmill_diameter = dw_endmill_diameter.GetNumber();
  int32_t hole_diameter = dw_hole_diameter.GetNumber() - endmill_diameter;
  int32_t stepover = dw_stepover.GetNumber();
  int32_t feed = dw_feed.GetNumber();

  // Find number of iterations to calculate memory needed for program
  uint32_t n = hole_diameter / stepover;
  // Calculate required memory(approximately)
  int32_t size = (5 + n) * 40;
  // And allocate it
  char *txt = ProgramSender::GetInstance().AllocateDataBuffer(size);

  if((txt != nullptr) && (dw_endmill_diameter.GetNumber() < dw_hole_diameter.GetNumber()))
  {
    // Clear enlarge progress and center
    int32_t enlarge_progress = 0;
    int32_t center = 0;

    // Switch to relative mode
    PrintStr(txt, size, "G91; Relative mode");

    // Generate cycle
    while((enlarge_progress + stepover * 2) < hole_diameter)
    {
      // Increase diameter by stepover
      enlarge_progress += stepover;
      // Find new I value for the first arc
      center = enlarge_progress / 2;
      // Arc(half circle) with enlarge operation.
      PrintStr(txt, size, direction ? "G3 I%li.%03i X%li.%03i F%li" : "G2 I%li.%03i X%li.%03i F%li", center/1000, center%1000, enlarge_progress/1000, enlarge_progress%1000, (feed * enlarge_progress) / (enlarge_progress + endmill_diameter));

      // Increase diameter by stepover
      enlarge_progress += stepover;
      // Find new I value for the second arc
      center = enlarge_progress / 2;
      // Arc(half circle)
      PrintStr(txt, size, direction ? "G3 I-%li.%03i X-%li.%03i F%li" : "G2 I-%li.%03i X-%li.%03i F%li", center/1000, center%1000, enlarge_progress/1000, enlarge_progress%1000, (feed * enlarge_progress) / (enlarge_progress + endmill_diameter));
    }

    // Find diameter of last enlarge arc - we need add radius of current circle and radius of desired circle.
    enlarge_progress = (enlarge_progress / 2) + (hole_diameter / 2);
    // Find new I value for the last enlarge arc
    center = enlarge_progress / 2;
    // Last enlarge arc(half circle)
    PrintStr(txt, size, direction ? "G3 I%li.%03i X%li.%03i F%li" : "G2 I%li.%03i X%li.%03i F%li", center/1000, center%1000, enlarge_progress/1000, enlarge_progress%1000, (feed * enlarge_progress) / (enlarge_progress + endmill_diameter));

    // Find new I value for the final circle
    center = hole_diameter / 2;
    // Do full circle with desired diameter
    PrintStr(txt, size, direction ? "G3 I-%li.%03i X-%li.%03i F%li" : "G2 I-%li.%03i X-%li.%03i F%li", center/1000, center%1000, hole_diameter/1000, hole_diameter%1000, (feed * hole_diameter) / (hole_diameter + endmill_diameter));
    PrintStr(txt, size, direction ? "G3 I%li.%03i X%li.%03i F%li" : "G2 I%li.%03i X%li.%03i F%li", center/1000, center%1000, hole_diameter/1000, hole_diameter%1000, (feed * hole_diameter) / (hole_diameter + endmill_diameter));

    // Return to center
    PrintStr(txt, size, "G0 X-%li.%03i", center/1000, center%1000);
  }

  return Result::RESULT_OK;
}

// ******************************************************************************
// ***   Private constructor   **************************************************
// ******************************************************************************
EnlargeGeneratorTab::EnlargeGeneratorTab() : left_btn(Application::GetInstance().GetLeftButton()) {};
