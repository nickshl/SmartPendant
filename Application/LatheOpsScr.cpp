//******************************************************************************
//  @file LatheOpsScr.cpp
//  @author Nicolai Shlapunov
//
//  @details LatheOpsScr: User LatheOpsScr Class, implementation
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
#include "LatheOpsScr.h"

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
// ***   PrintCmdG0   **********************************************************
// *****************************************************************************
static inline void PrintCmdG0(char*& buf, int32_t& len, const char* axis, int32_t position)
{
  PrintStr(buf, len, "G0 %s%s%li.%03i", axis, (position < 0) ? "-" : "", abs(position/1000), abs(position%1000));
}

// *****************************************************************************
// ***   PrintCmdG1   **********************************************************
// *****************************************************************************
static inline void PrintCmdG1(char*& buf, int32_t& len, const char* axis, int32_t position, int32_t speed)
{
  PrintStr(buf, len, "G1 %s%s%li.%03i F%li", axis, (position < 0) ? "-" : "", abs(position/1000), abs(position%1000), speed);
}

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
LatheOpsScr& LatheOpsScr::GetInstance()
{
  static LatheOpsScr LatheOpsScr;
  return LatheOpsScr;
}

// *****************************************************************************
// ***   LatheOpsScr Setup   ***************************************************
// *****************************************************************************
Result LatheOpsScr::Setup(int32_t y, int32_t height)
{
  // Tabs for screens(first call to set parameters)
  tabs.SetParams(0, y, DisplayDrv::GetInstance().GetScreenW(), 40, Tabs::MAX_TABS);
  // Screens & Captions
  tabs_cnt = 0u; // Clear tabs since Setup() can be called multiple times
  // Turn tab
  tabs.SetText(tabs_cnt, "Turn", nullptr, Font_10x18::GetInstance());
  tab[tabs_cnt++] = &TurnGeneratorTab::GetInstance();
  // Face tab
  tabs.SetText(tabs_cnt, "Face", nullptr, Font_10x18::GetInstance());
  tab[tabs_cnt++] = &FaceGeneratorTab::GetInstance();
  // Cut tab
  tabs.SetText(tabs_cnt, "Cut", nullptr, Font_10x18::GetInstance());
  tab[tabs_cnt++] = &CutGeneratorTab::GetInstance();
  // Tabs for screens(second call to resize to actual numbers of pages)
  tabs.SetParams(0, y, DisplayDrv::GetInstance().GetScreenW(), 40, tabs_cnt);
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
Result LatheOpsScr::Show()
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
Result LatheOpsScr::Hide()
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
Result LatheOpsScr::TimerExpired(uint32_t interval)
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
Result LatheOpsScr::ProcessCallback(const void* ptr)
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
Result LatheOpsScr::ProcessButtonCallback(LatheOpsScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    LatheOpsScr& ths = *obj_ptr;
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
void LatheOpsScr::ChangeTab(uint8_t tabn)
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
LatheOpsScr::LatheOpsScr() : right_btn(Application::GetInstance().GetRightButton()) {};

// ******************************************************************************
// ******************************************************************************
// ***   TURN GENERATOR TAB   ***************************************************
// ******************************************************************************
// ******************************************************************************

// ******************************************************************************
// ***   Get Instance   *********************************************************
// ******************************************************************************
TurnGeneratorTab& TurnGeneratorTab::GetInstance()
{
  static TurnGeneratorTab turn_generator_tab;
  return turn_generator_tab;
}

// ******************************************************************************
// ***   TurnGeneratorTab Setup   ***********************************************
// ******************************************************************************
Result TurnGeneratorTab::Setup(int32_t y, int32_t height)
{
  // Fill menu_items
  for(uint32_t i = 0u; i < NumberOf(menu_items); i++)
  {
    menu_items[i].text = str[i];
    menu_items[i].n = sizeof(str[i]);
  }
  // Set callback
  menu.SetCallback(AppTask::GetCurrent(), this, reinterpret_cast<CallbackPtr>(ProcessMenuCallback), nullptr);
  // Setup menu
  menu.Setup(0, y, DisplayDrv::GetInstance().GetScreenW(), height);

  // Create and set
  UpdateStrings();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result TurnGeneratorTab::Show()
{
  // Show menu
  menu.Show(100);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result TurnGeneratorTab::Hide()
{
  // Hide change box if it on the screen now
  change_box.Hide();
  // Hide menu
  menu.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result TurnGeneratorTab::TimerExpired(uint32_t interval)
{
  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result TurnGeneratorTab::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  // Process change box callback
  if(ptr == &change_box)
  {
    // Save value
    values[change_box.GetId()] = change_box.GetValue();
    // Update strings on display
    UpdateStrings();
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ProcessMenuCallback function   *******************************
// *****************************************************************************
Result TurnGeneratorTab::ProcessMenuCallback(TurnGeneratorTab* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    TurnGeneratorTab& ths = *obj_ptr;
    // Convert pointer to index
    uint32_t idx = (uint32_t)ptr;

    const char* units = nullptr;
    uint32_t precision = 0;

    if((idx == TURN_LENGTH) || (idx == TURN_DIAMETER) || (idx == TURN_STEP) || (idx == TURN_FINE_STEP))
    {
      units = ths.grbl_comm.GetReportUnits();
      precision = ths.grbl_comm.GetUnitsPrecision();
    }
    else if((idx == TURN_SPEED) || (idx == TURN_FINE_SPEED))
    {
      units = ths.grbl_comm.GetReportSpeedUnits();
      precision = 0;
    }
    else if(idx == TURN_GENERATE)
    {
      // If we already run program
      // We can start probing only in IDLE state and if probing isn't started yet
      if(ths.grbl_comm.GetState() != GrblComm::IDLE)
      {
        result = Result::ERR_UNHANDLED_REQUEST; // For Application to handle it
      }
      else
      {
        // Generate GCode for ProgramSender
        ths.GenerateGcode();
        // Switch to the Program Sender screen
        Application::GetInstance().ChangeScreen(ProgramSender::GetInstance());
      }
    }
    else
    {
      ; // Do nothing - MISRA rule
    }

    // Show change box only if index is found
    if(units != nullptr)
    {
      // Setup object to change numerical parameters, title scale set to 1
      ths.change_box.Setup(ths.menu_strings[idx], units, ths.values[idx], -10000000, 10000000, precision);
      // Set AppTask
      ths.change_box.SetCallback(AppTask::GetCurrent());
      // Save axis index as ID
      ths.change_box.SetId(idx);
      // Show change box
      ths.change_box.Show(10000u);
      // Update string on display
      ths.UpdateStrings();
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: UpdateStrings function   *************************************
// *****************************************************************************
void TurnGeneratorTab::UpdateStrings(void)
{
  char tmp_str[16u] = {0};

  menu.CreateString(menu_items[TURN_LENGTH],     menu_strings[TURN_LENGTH],     grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[TURN_LENGTH],     grbl_comm.GetUnitsScaler(), grbl_comm.GetReportUnits()));
  menu.CreateString(menu_items[TURN_DIAMETER],   menu_strings[TURN_DIAMETER],   grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[TURN_DIAMETER],   grbl_comm.GetUnitsScaler(), grbl_comm.GetReportUnits()));
  menu.CreateString(menu_items[TURN_STEP],       menu_strings[TURN_STEP],       grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[TURN_STEP],       grbl_comm.GetUnitsScaler(), grbl_comm.GetReportUnits()));
  menu.CreateString(menu_items[TURN_SPEED],      menu_strings[TURN_SPEED],      grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[TURN_SPEED],      grbl_comm.GetSpeedScaler(), grbl_comm.GetReportSpeedUnits()));
  menu.CreateString(menu_items[TURN_FINE_STEP],  menu_strings[TURN_FINE_STEP],  grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[TURN_FINE_STEP],  grbl_comm.GetUnitsScaler(), grbl_comm.GetReportUnits()));
  menu.CreateString(menu_items[TURN_FINE_SPEED], menu_strings[TURN_FINE_SPEED], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[TURN_FINE_SPEED], grbl_comm.GetSpeedScaler(), grbl_comm.GetReportSpeedUnits()));
  menu.CreateString(menu_items[TURN_GENERATE],   menu_strings[TURN_GENERATE], "");
}

// ******************************************************************************
// ***   Private: GenerateGcode function   **************************************
// ******************************************************************************
Result TurnGeneratorTab::GenerateGcode()
{
  // Get start Z position
  int32_t start_z_position = grbl_comm.GetAxisPosition(GrblComm::AXIS_Z);
  // Calculate end Z position
  int32_t end_z_position = start_z_position - values[TURN_LENGTH];
  // Get current diameter. If machine in radius mode - multiply axis position by 2.
  int32_t current_diameter = grbl_comm.GetAxisPosition(GrblComm::AXIS_X) * (grbl_comm.IsLatheDiameterMode() ? 1 : 2);
  // Find cut distance. Difference in diameters divided by 2.
  int32_t cut_distance = abs(current_diameter - values[TURN_DIAMETER]) / 2;
  // Find number of rough passes
  int32_t rough_pass_cnt = ((cut_distance - values[TURN_FINE_STEP]) / values[TURN_STEP] + 1);
  // Find rough step to keep desired thickness of finishing pass
  int32_t rough_pass_distance = (cut_distance - values[TURN_FINE_STEP]) / rough_pass_cnt;

  // If we need to cut distance less than finishing pass - there shouldn't be any roughing passes
  if(cut_distance <= values[TURN_FINE_STEP])
  {
    rough_pass_cnt = 0;
  }

  // Find if we want to make outside or inside turn
  bool is_outside = (current_diameter > values[TURN_DIAMETER]);

  // Current X axis position
  int32_t x_position = current_diameter / 2;

  // Calculate size (approx. Number of steps * 3 lines per step * 32 bytes per line)
  int32_t size = (rough_pass_cnt + 2) * 4 * 32;
  // Allocate buffer
  char *txt = ProgramSender::GetInstance().AllocateDataBuffer(size);

  // If allocation is successful and we can determinate if it outside or inside turn - generate G-code
  if((txt != nullptr) && (current_diameter != values[TURN_DIAMETER]))
  {
    PrintStr(txt, size, "G90; Absolute mode");
    PrintStr(txt, size, "G8; Radius mode");

    for(int32_t i = 0u; i < rough_pass_cnt; i++)
    {
      // Cutting radius
      x_position += is_outside ? -rough_pass_distance : rough_pass_distance;
      // Move tool to cut position
      PrintCmdG1(txt, size, "X", x_position, values[TURN_SPEED]);
      // Make a pass
      PrintCmdG1(txt, size, "Z", end_z_position, values[TURN_SPEED]);
      // Move tool away from part(1 mm clearence)
      PrintCmdG0(txt, size, "X", x_position + (is_outside ? 1000 : -1000));
      // Return tool to start point
      PrintCmdG0(txt, size, "Z", start_z_position);
    }
    // Final pass
    x_position = values[TURN_DIAMETER] / 2;
    // Move tool to cut position
    PrintCmdG1(txt, size, "X", x_position, values[TURN_FINE_SPEED]);
    // Make a pass
    PrintCmdG1(txt, size, "Z", end_z_position, values[TURN_FINE_SPEED]);
    // Move tool away from part(1 mm clearence)
    PrintCmdG0(txt, size, "X", x_position + (is_outside ? 1000 : -1000));
    // Return tool to start point
    PrintCmdG0(txt, size, "Z", start_z_position);
    // Move tool to desired diameter
    PrintCmdG1(txt, size, "X", x_position, values[TURN_FINE_SPEED]);
}

  return Result::RESULT_OK;
}

// ******************************************************************************
// ***   Private constructor   **************************************************
// ******************************************************************************
TurnGeneratorTab::TurnGeneratorTab() : menu(menu_items, NumberOf(menu_items)), change_box(Application::GetInstance().GetChangeValueBox()) {};

// ******************************************************************************
// ******************************************************************************
// ***   FACE GENERATOR TAB   ***************************************************
// ******************************************************************************
// ******************************************************************************

// ******************************************************************************
// ***   Get Instance   *********************************************************
// ******************************************************************************
FaceGeneratorTab& FaceGeneratorTab::GetInstance()
{
  static FaceGeneratorTab face_generator_tab;
  return face_generator_tab;
}

// ******************************************************************************
// ***   FaceGeneratorTab Setup   ***********************************************
// ******************************************************************************
Result FaceGeneratorTab::Setup(int32_t y, int32_t height)
{
  // Fill menu_items
  for(uint32_t i = 0u; i < NumberOf(menu_items); i++)
  {
    menu_items[i].text = str[i];
    menu_items[i].n = sizeof(str[i]);
  }
  // Set callback
  menu.SetCallback(AppTask::GetCurrent(), this, reinterpret_cast<CallbackPtr>(ProcessMenuCallback), nullptr);
  // Setup menu
  menu.Setup(0, y, DisplayDrv::GetInstance().GetScreenW(), height);

  // Create and set
  UpdateStrings();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result FaceGeneratorTab::Show()
{
  // Show menu
  menu.Show(100);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result FaceGeneratorTab::Hide()
{
  // Hide change box if it on the screen now
  change_box.Hide();
  // Hide menu
  menu.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result FaceGeneratorTab::TimerExpired(uint32_t interval)
{
  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result FaceGeneratorTab::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  // Process change box callback
  if(ptr == &change_box)
  {
    // Save value
    values[change_box.GetId()] = change_box.GetValue();
    // Update strings on display
    UpdateStrings();
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ProcessMenuCallback function   *******************************
// *****************************************************************************
Result FaceGeneratorTab::ProcessMenuCallback(FaceGeneratorTab* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    FaceGeneratorTab& ths = *obj_ptr;
    // Convert pointer to index
    uint32_t idx = (uint32_t)ptr;

    const char* units = nullptr;
    uint32_t precision = 0;

    if((idx == FACE_LENGTH) || (idx == FACE_DIAMETER) || (idx == FACE_STEP) || (idx == FACE_FINE_STEP))
    {
      units = ths.grbl_comm.GetReportUnits();
      precision = ths.grbl_comm.GetUnitsPrecision();
    }
    else if((idx == FACE_SPEED) || (idx == FACE_FINE_SPEED))
    {
      units = ths.grbl_comm.GetReportSpeedUnits();
      precision = 0;
    }
    else if(idx == FACE_GENERATE)
    {
      // If we already run program
      // We can start probing only in IDLE state and if probing isn't started yet
      if(ths.grbl_comm.GetState() != GrblComm::IDLE)
      {
        result = Result::ERR_UNHANDLED_REQUEST; // For Application to handle it
      }
      else
      {
        // Generate GCode for ProgramSender
        ths.GenerateGcode();
        // Switch to the Program Sender screen
        Application::GetInstance().ChangeScreen(ProgramSender::GetInstance());
      }
    }
    else
    {
      ; // Do nothing - MISRA rule
    }

    // Show change box only if index is found
    if(units != nullptr)
    {
      // Setup object to change numerical parameters, title scale set to 1
      ths.change_box.Setup(ths.menu_strings[idx], units, ths.values[idx], -10000000, 10000000, precision);
      // Set AppTask
      ths.change_box.SetCallback(AppTask::GetCurrent());
      // Save axis index as ID
      ths.change_box.SetId(idx);
      // Show change box
      ths.change_box.Show(10000u);
      // Update string on display
      ths.UpdateStrings();
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: UpdateStrings function   *************************************
// *****************************************************************************
void FaceGeneratorTab::UpdateStrings(void)
{
  char tmp_str[16u] = {0};

  menu.CreateString(menu_items[FACE_LENGTH],     menu_strings[FACE_LENGTH],     grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[FACE_LENGTH],     grbl_comm.GetUnitsScaler(), grbl_comm.GetReportUnits()));
  menu.CreateString(menu_items[FACE_DIAMETER],   menu_strings[FACE_DIAMETER],   grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[FACE_DIAMETER],   grbl_comm.GetUnitsScaler(), grbl_comm.GetReportUnits()));
  menu.CreateString(menu_items[FACE_STEP],       menu_strings[FACE_STEP],       grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[FACE_STEP],       grbl_comm.GetUnitsScaler(), grbl_comm.GetReportUnits()));
  menu.CreateString(menu_items[FACE_SPEED],      menu_strings[FACE_SPEED],      grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[FACE_SPEED],      grbl_comm.GetSpeedScaler(), grbl_comm.GetReportSpeedUnits()));
  menu.CreateString(menu_items[FACE_FINE_STEP],  menu_strings[FACE_FINE_STEP],  grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[FACE_FINE_STEP],  grbl_comm.GetUnitsScaler(), grbl_comm.GetReportUnits()));
  menu.CreateString(menu_items[FACE_FINE_SPEED], menu_strings[FACE_FINE_SPEED], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[FACE_FINE_SPEED], grbl_comm.GetSpeedScaler(), grbl_comm.GetReportSpeedUnits()));
  menu.CreateString(menu_items[FACE_GENERATE],   menu_strings[FACE_GENERATE], "");
}

// ******************************************************************************
// ***   Private: GenerateGcode function   **************************************
// ******************************************************************************
Result FaceGeneratorTab::GenerateGcode()
{
  // Get start X position
  int32_t start_x_position = grbl_comm.GetAxisPosition(GrblComm::AXIS_X) / (grbl_comm.IsLatheDiameterMode() ? 2 : 1);
  // Calculate end X position
  int32_t end_x_position = values[FACE_DIAMETER] / 2;
  // Find number of rough passes
  int32_t rough_pass_cnt = ((values[FACE_LENGTH] - values[FACE_FINE_STEP]) / values[FACE_STEP] + 1);
  // Find rough step to keep desired thickness of finishing pass
  int32_t rough_pass_distance = (values[FACE_LENGTH] - values[FACE_FINE_STEP]) / rough_pass_cnt;

  // If we need to cut distance less than finishing pass - there shouldn't be any roughing passes
  if(values[FACE_LENGTH] <= values[FACE_FINE_STEP])
  {
    rough_pass_cnt = 0;
  }

  // Get start Z position
  int32_t z_position = grbl_comm.GetAxisPosition(GrblComm::AXIS_Z);

  // Calculate size (approx. Number of steps * 3 lines per step * 32 bytes per line)
  int32_t size = (rough_pass_cnt + 2) * 4 * 32;
  // Allocate buffer
  char *txt = ProgramSender::GetInstance().AllocateDataBuffer(size);

  // If allocation is successful - generate G-code
  if(txt != nullptr)
  {
    PrintStr(txt, size, "G90; Absolute mode");
    PrintStr(txt, size, "G8; Radius mode");

    for(int32_t i = 0u; i < rough_pass_cnt; i++)
    {
      // Cutting radius
      z_position -= rough_pass_distance;
      // Move tool to cut position
      PrintCmdG1(txt, size, "Z", z_position, values[FACE_SPEED]);
      // Make a pass
      PrintCmdG1(txt, size, "X", end_x_position, values[FACE_SPEED]);
      // Move tool away from part(1 mm clearence)
      PrintCmdG0(txt, size, "Z", z_position + 1000);
      // Return tool to start point
      PrintCmdG0(txt, size, "X", start_x_position);
    }
    // Final pass
    z_position = grbl_comm.GetAxisPosition(GrblComm::AXIS_Z) - values[FACE_LENGTH];
    // Move tool to cut position
    PrintCmdG1(txt, size, "Z", z_position, values[FACE_FINE_SPEED]);
    // Make a pass
    PrintCmdG1(txt, size, "X", end_x_position, values[FACE_FINE_SPEED]);
    // Move tool away from part(1 mm clearence)
    PrintCmdG0(txt, size, "Z", z_position + 1000);
    // Return tool to start point
    PrintCmdG0(txt, size, "X", start_x_position);
    // Move tool to desired diameter
    PrintCmdG1(txt, size, "Z", z_position, values[FACE_FINE_SPEED]);
  }

  return Result::RESULT_OK;
}

// ******************************************************************************
// ***   Private constructor   **************************************************
// ******************************************************************************
FaceGeneratorTab::FaceGeneratorTab() : menu(menu_items, NumberOf(menu_items)), change_box(Application::GetInstance().GetChangeValueBox()) {};

// ******************************************************************************
// ******************************************************************************
// ***   Cut GENERATOR TAB   ****************************************************
// ******************************************************************************
// ******************************************************************************

// ******************************************************************************
// ***   Get Instance   *********************************************************
// ******************************************************************************
CutGeneratorTab& CutGeneratorTab::GetInstance()
{
  static CutGeneratorTab cut_generator_tab;
  return cut_generator_tab;
}

// ******************************************************************************
// ***   CutGeneratorTab Setup   ************************************************
// ******************************************************************************
Result CutGeneratorTab::Setup(int32_t y, int32_t height)
{
  // Fill menu_items
  for(uint32_t i = 0u; i < NumberOf(menu_items); i++)
  {
    menu_items[i].text = str[i];
    menu_items[i].n = sizeof(str[i]);
  }
  // Set callback
  menu.SetCallback(AppTask::GetCurrent(), this, reinterpret_cast<CallbackPtr>(ProcessMenuCallback), nullptr);
  // Setup menu
  menu.Setup(0, y, DisplayDrv::GetInstance().GetScreenW(), height);

  // Create and set
  UpdateStrings();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result CutGeneratorTab::Show()
{
  // Show menu
  menu.Show(100);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result CutGeneratorTab::Hide()
{
  // Hide change box if it on the screen now
  change_box.Hide();
  // Hide menu
  menu.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result CutGeneratorTab::TimerExpired(uint32_t interval)
{
  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result CutGeneratorTab::ProcessCallback(const void* ptr)
{
  Result result = Result::RESULT_OK;

  // Process change box callback
  if(ptr == &change_box)
  {
    // Save value
    values[change_box.GetId()] = change_box.GetValue();
    // Update strings on display
    UpdateStrings();
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ProcessMenuCallback function   *******************************
// *****************************************************************************
Result CutGeneratorTab::ProcessMenuCallback(CutGeneratorTab* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    CutGeneratorTab& ths = *obj_ptr;
    // Convert pointer to index
    uint32_t idx = (uint32_t)ptr;

    const char* units = nullptr;
    uint32_t precision = 0;

    if((idx == CUT_DIAMETER) || (idx == CUT_WIDTH) || (idx == CUT_STEP))
    {
      units = ths.grbl_comm.GetReportUnits();
      precision = ths.grbl_comm.GetUnitsPrecision();
    }
    else if(idx == CUT_SPEED)
    {
      units = ths.grbl_comm.GetReportSpeedUnits();
      precision = 0;
    }
    else if(idx == CUT_GENERATE)
    {
      // If we already run program
      // We can start probing only in IDLE state and if probing isn't started yet
      if(ths.grbl_comm.GetState() != GrblComm::IDLE)
      {
        result = Result::ERR_UNHANDLED_REQUEST; // For Application to handle it
      }
      else
      {
        // Generate GCode for ProgramSender
        ths.GenerateGcode();
        // Switch to the Program Sender screen
        Application::GetInstance().ChangeScreen(ProgramSender::GetInstance());
      }
    }
    else
    {
      ; // Do nothing - MISRA rule
    }

    // Show change box only if index is found
    if(units != nullptr)
    {
      // Setup object to change numerical parameters, title scale set to 1
      ths.change_box.Setup(ths.menu_strings[idx], units, ths.values[idx], -10000000, 10000000, precision);
      // Set AppTask
      ths.change_box.SetCallback(AppTask::GetCurrent());
      // Save axis index as ID
      ths.change_box.SetId(idx);
      // Show change box
      ths.change_box.Show(10000u);
      // Update string on display
      ths.UpdateStrings();
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: UpdateStrings function   *************************************
// *****************************************************************************
void CutGeneratorTab::UpdateStrings(void)
{
  char tmp_str[16u] = {0};

  menu.CreateString(menu_items[CUT_DIAMETER], menu_strings[CUT_DIAMETER], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[CUT_DIAMETER], grbl_comm.GetUnitsScaler(), grbl_comm.GetReportUnits()));
  menu.CreateString(menu_items[CUT_WIDTH],    menu_strings[CUT_WIDTH],    grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[CUT_WIDTH],    grbl_comm.GetUnitsScaler(), grbl_comm.GetReportUnits()));
  menu.CreateString(menu_items[CUT_STEP],     menu_strings[CUT_STEP],     grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[CUT_STEP],     grbl_comm.GetUnitsScaler(), grbl_comm.GetReportUnits()));
  menu.CreateString(menu_items[CUT_SPEED],    menu_strings[CUT_SPEED],    grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), values[CUT_SPEED],    grbl_comm.GetSpeedScaler(), grbl_comm.GetReportSpeedUnits()));
  menu.CreateString(menu_items[CUT_GENERATE], menu_strings[CUT_GENERATE], "");
}

// ******************************************************************************
// ***   Private: GenerateGcode function   **************************************
// ******************************************************************************
Result CutGeneratorTab::GenerateGcode()
{
  // Get start X position
  int32_t start_x_position = grbl_comm.GetAxisPosition(GrblComm::AXIS_X) / (grbl_comm.IsLatheDiameterMode() ? 2 : 1);
  // Calculate end X position
  int32_t end_x_position = values[CUT_DIAMETER] / 2;
  // Current position for X
  int32_t current_x_position = start_x_position;
  // Find number of passes
  int32_t pass_cnt = (start_x_position - end_x_position) / values[CUT_STEP] + 1;
  // Find step distance
  int32_t pass_distance = (start_x_position - end_x_position) / pass_cnt;

  // Get start Z position
  int32_t z_position = grbl_comm.GetAxisPosition(GrblComm::AXIS_Z);

  // Calculate size (approx. Number of steps * 3 lines per step * 32 bytes per line)
  int32_t size = (pass_cnt + 2) * 4 * 32;
  // Allocate buffer
  char *txt = ProgramSender::GetInstance().AllocateDataBuffer(size);

  // If allocation is successful - generate G-code
  if(txt != nullptr)
  {
    PrintStr(txt, size, "G90; Absolute mode");
    PrintStr(txt, size, "G8; Radius mode");

    for(int32_t i = 0u; i < pass_cnt; i++)
    {
      // For each additional cut make fast dive(1 mm clearance) to save time
      if(i != 0) PrintCmdG0(txt, size, "X", current_x_position + 1000);
      // Cutting radius
      current_x_position -= pass_distance;
      // Make a pass
      PrintCmdG1(txt, size, "X", current_x_position, values[CUT_SPEED]);
      // Retract tool
      PrintCmdG0(txt, size, "X", start_x_position);
      // Second side pass only if width is set
      if(values[CUT_WIDTH] != 0)
      {
        // Move tool to different Z location
        PrintCmdG0(txt, size, "Z", z_position + values[CUT_WIDTH]);
        // For each additional cut make fast dive(1 mm clearance) to save time
        if(i != 0) PrintCmdG0(txt, size, "X", current_x_position + 1000);
        // Make a pass
        PrintCmdG1(txt, size, "X", current_x_position, values[CUT_SPEED]);
        // Retract tool
        PrintCmdG0(txt, size, "X", start_x_position);
        // Move tool to initial Z location
        PrintCmdG0(txt, size, "Z", z_position);
      }
    }
  }

  return Result::RESULT_OK;
}

// ******************************************************************************
// ***   Private constructor   **************************************************
// ******************************************************************************
CutGeneratorTab::CutGeneratorTab() : menu(menu_items, NumberOf(menu_items)), change_box(Application::GetInstance().GetChangeValueBox()) {};

