//******************************************************************************
//  @file SettingsScr.cpp
//  @author Nicolai Shlapunov
//
//  @details SettingsScr: User SettingsScr Class, implementation
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
#include "SettingsScr.h"

#include "Application.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
SettingsScr& SettingsScr::GetInstance()
{
  static SettingsScr settings_scr;
  return settings_scr;
}

// *****************************************************************************
// ***   SettingsScr Setup   ***************************************************
// *****************************************************************************
Result SettingsScr::Setup(int32_t y, int32_t height)
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
  menu.Setup(0, y, display_drv.GetScreenW(), height);

  // Create and set
  UpdateStrings();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result SettingsScr::Show()
{
  // Show menu
  menu.Show(100);

  // Update string on display
  UpdateStrings();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result SettingsScr::Hide()
{
  // Hide menu
  menu.Hide();

  // Save data into EEPROM after exit the screen
  nvm.WriteData();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   TimerExpired function   ***********************************************
// *****************************************************************************
Result SettingsScr::TimerExpired(uint32_t interval)
{
  // Return ok - we don't check semaphore give error, because we don't need to.
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ProcessCallback function   ********************************************
// *****************************************************************************
Result SettingsScr::ProcessCallback(const void* ptr)
{
  // Process change box callback
  if(ptr == &change_box)
  {
    // Save value
    nvm.SetValue((NVM::Parameters)change_box.GetId(), grbl_comm.ConvertUnitsToMetric(change_box.GetValue()));
    // Update strings on display
    UpdateStrings();
  }

  // Always good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Private: ProcessMenuCallback function   *******************************
// *****************************************************************************
Result SettingsScr::ProcessMenuCallback(SettingsScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    SettingsScr& ths = *obj_ptr;
    // Convert pointer to index
    uint32_t idx = (uint32_t)ptr;

    if(idx == TX_CONTROL)
    {
      uint8_t val = ths.nvm.GetCtrlTx() + 1u;    // Get current value and increment it by 1
      if(val >= GrblComm::CTRL_TX_CNT) val = 0u; // Check overflow
      ths.nvm.SetCtrlTx(val);                    // Store new value
    }
    else if(idx == SCREEN_INVERT)
    {
      ths.nvm.SetDisplayInvert(!ths.nvm.GetDisplayInvert());
      ths.display_drv.InvertDisplay(ths.nvm.GetDisplayInvert());
    }
    else if(idx == JOGGING_SPEED)
    {
      ths.nvm.SetFastJogging(!ths.nvm.GetFastJogging());
      //ths.display_drv.InvertDisplay(ths.nvm.GetDisplayInvert());
    }
    else
    {
      const char* units = nullptr;
      uint32_t precision = 0;

      if(idx == PROBE_BALL_TIP)
      {
        units = ths.grbl_comm.GetReportUnits();
        precision = ths.grbl_comm.GetUnitsPrecision();
      }
      else if((idx == PROBE_SEARCH_FEED) || (idx == PROBE_LOCK_FEED))
      {
        units = ths.grbl_comm.GetReportSpeedUnits();
        precision = 0;
      }
      else
      {
        ; // Do nothing - MISRA rule
      }

      // Show change box only if index is found
      if(units != nullptr)
      {
        // Setup object to change numerical parameters, title scale set to 1
        ths.change_box.Setup(ths.menu_strings[idx], units, ths.grbl_comm.ConvertMetricToUnits(ths.nvm.GetValue((NVM::Parameters)idx)) , 1, 10000, precision, 1u);
        // Set AppTask
        ths.change_box.SetCallback(AppTask::GetCurrent());
        // Save axis index as ID
        ths.change_box.SetId(idx);
        // Show change box
        ths.change_box.Show(10000u);
      }
    }

    // Update string on display
    ths.UpdateStrings();

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: UpdateStrings function   *************************************
// *****************************************************************************
void SettingsScr::UpdateStrings(void)
{
  char tmp_str[16u] = {0};
  menu.CreateString(menu_items[TX_CONTROL],        menu_strings[TX_CONTROL], (nvm.GetCtrlTx() == GrblComm::CTRL_GPIO_PIN) ? "dedicated pin" : (nvm.GetCtrlTx() == GrblComm::CTRL_SW_COMMAND) ? "sw command" : (nvm.GetCtrlTx() == GrblComm::CTRL_PIN_AND_SW_CMD) ? "pin & sw cmd": "full control");
  menu.CreateString(menu_items[SCREEN_INVERT],     menu_strings[SCREEN_INVERT], nvm.GetDisplayInvert() ? "inverted" : "normal");
  menu.CreateString(menu_items[PROBE_SEARCH_FEED], menu_strings[PROBE_SEARCH_FEED], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), grbl_comm.ConvertMetricToUnits(nvm.GetValue(NVM::PROBE_SEARCH_FEED)), grbl_comm.GetSpeedScaler(), grbl_comm.GetReportSpeedUnits()));
  menu.CreateString(menu_items[PROBE_LOCK_FEED],   menu_strings[PROBE_LOCK_FEED],   grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), grbl_comm.ConvertMetricToUnits(nvm.GetValue(NVM::PROBE_LOCK_FEED)), grbl_comm.GetSpeedScaler(), grbl_comm.GetReportSpeedUnits()));
  menu.CreateString(menu_items[PROBE_BALL_TIP],    menu_strings[PROBE_BALL_TIP],    grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), grbl_comm.ConvertMetricToUnits(nvm.GetValue(NVM::PROBE_BALL_TIP)), grbl_comm.GetUnitsScaler(), grbl_comm.GetReportUnits()));
  menu.CreateString(menu_items[JOGGING_SPEED],     menu_strings[JOGGING_SPEED], nvm.GetFastJogging() ? "fast" : "normal");
}

// ******************************************************************************
// ***   Private constructor   **************************************************
// ******************************************************************************
SettingsScr::SettingsScr() : menu(menu_items, NumberOf(menu_items)), change_box(Application::GetInstance().GetChangeValueBox()) {};
