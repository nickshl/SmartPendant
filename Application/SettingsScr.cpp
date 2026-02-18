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
  // Tabs for screens
  tabs.SetParams(0, y, DisplayDrv::GetInstance().GetScreenW(), 40, 3u);
  // General settings tab
  tabs.SetText(0u, "GENERAL", nullptr, Font_10x18::GetInstance());
  // MPG settings tab
  tabs.SetText(1u, "MPG", nullptr, Font_10x18::GetInstance());
  // Probe settings tab
  tabs.SetText(2u, "PROBE", nullptr, Font_10x18::GetInstance());
  // Set callback
  tabs.SetCallback(AppTask::GetCurrent());

  // Fill menu_items
  for(uint32_t i = 0u; i < NumberOf(menu_items); i++)
  {
    menu_items[i].text = str[i];
    menu_items[i].n = sizeof(str[i]);
  }
  // Set callback
  menu.SetCallback(AppTask::GetCurrent(), this, reinterpret_cast<CallbackPtr>(ProcessMenuCallback), nullptr);
  // Setup menu
  menu.Setup(0, y + tabs.GetHeight(), display_drv.GetScreenW(), height - tabs.GetHeight());

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
  // Show tabs
  tabs.Show(2000);

  if(!Application::GetInstance().GetMsgBox().IsShow())
  {
    // Show menu
    menu.Show(100);
  }

  // Update string on display
  UpdateStrings();

  // Set callback handler for left and right buttons
  InputDrv::GetInstance().AddButtonsCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this, InputDrv::BTNM_LEFT_DOWN | InputDrv::BTNM_RIGHT_DOWN, btn_cble);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result SettingsScr::Hide()
{
  // Delete buttons callback handler
  InputDrv::GetInstance().DeleteButtonsCallbackHandler(btn_cble);

  // Hide menu
  menu.Hide();
  // Show tabs
  tabs.Hide();

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
  // Process tabs
  if(ptr == &tabs)
  {
    // Populate menu with global variables
    UpdateStrings();
    menu.Show(100);
  }
  // Process change box callback
  else if(ptr == &change_box)
  {
    // Update variable and strings only if user pressed "OK" button
    if(change_box.GetResult())
    {
      // MPG tab
      if(tabs.GetSelectedTab() == MPG_TAB)
      {
        // Save value as is since we have separate values for metric and imperial
        nvm.SetValue((NVM::Parameters)(change_box.GetId() + NVM::MPG_METRIC_FEED_1), change_box.GetValue());
      }
      // Probe tab
      else if(tabs.GetSelectedTab() == PROBE_TAB)
      {
        // Save value - probe parameters always saved as metric
        nvm.SetValue((NVM::Parameters)(change_box.GetId() + NVM::PROBE_SEARCH_FEED), grbl_comm.ConvertUnitsToMetric(change_box.GetValue()));
      }
      else
      {
        ; // Do nothing - MISRA rule
      }
      // Update strings on display
      UpdateStrings();
    }
  }
  // Process message box with an error
  if(ptr == &Application::GetInstance().GetMsgBox())
  {
    // Show menu
    menu.Show(100);
  }
  else
  {
    ; // Do nothing - MISRA rule
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

    // General tab
    if(ths.tabs.GetSelectedTab() == GENERAL_TAB)
    {
      // Convert menu index to NVM index
      uint32_t nvm_idx = idx + NVM::TX_CONTROL;

      if(nvm_idx == NVM::TX_CONTROL)
      {
        uint8_t val = ths.nvm.GetCtrlTx() + 1u;    // Get current value and increment it by 1
        if(val >= GrblComm::CTRL_TX_CNT) val = 0u; // Check overflow
        ths.nvm.SetCtrlTx(val);                    // Store new value
      }
      else if(nvm_idx == NVM::SCREEN_INVERT)
      {
        ths.nvm.SetValue(NVM::SCREEN_INVERT, !ths.nvm.GetValue(NVM::SCREEN_INVERT));
        ths.display_drv.InvertDisplay(ths.nvm.GetValue(NVM::SCREEN_INVERT));
      }
      else if(nvm_idx == NVM::AUTO_MPG_ON_START)
      {
        ths.nvm.SetValue(NVM::AUTO_MPG_ON_START, !ths.nvm.GetValue(NVM::AUTO_MPG_ON_START));
      }
      else if(nvm_idx == NVM::SAVE_SCRIPT_RESULT)
      {
        ths.nvm.SetValue(NVM::SAVE_SCRIPT_RESULT, !ths.nvm.GetValue(NVM::SAVE_SCRIPT_RESULT));
      }
      else
      {
        ; // Do nothing - MISRA rule
      }
    }
    // MPG tab
    else if(ths.tabs.GetSelectedTab() == MPG_TAB)
    {
      // Convert menu index to NVM index
      uint32_t nvm_idx = idx + NVM::MPG_METRIC_FEED_1;
      // Units and precision variables
      const char* units = nullptr;
      uint32_t precision = 0;

      if((nvm_idx >= NVM::MPG_METRIC_FEED_1) && (nvm_idx <= NVM::MPG_METRIC_FEED_4))
      {
        units = ths.grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_METRIC);
        precision = ths.grbl_comm.GetUnitsPrecision(GrblComm::MEASUREMENT_SYSTEM_METRIC);
      }
      else if((nvm_idx >= NVM::MPG_IMPERIAL_FEED_1) && (nvm_idx <= NVM::MPG_IMPERIAL_FEED_4))
      {
        units = ths.grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_IMPERIAL);
        precision = ths.grbl_comm.GetUnitsPrecision(GrblComm::MEASUREMENT_SYSTEM_IMPERIAL);
      }
      else if((nvm_idx >= NVM::MPG_ROTARY_FEED_1) && (nvm_idx <= NVM::MPG_ROTARY_FEED_4))
      {
        units = ths.grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_ROTARY);
        precision = ths.grbl_comm.GetUnitsPrecision(GrblComm::MEASUREMENT_SYSTEM_ROTARY);
      }
      else
      {
        ; // Do nothing - MISRA rule
      }

      // Show change box only if index is found
      if(units != nullptr)
      {
        // Setup object to change numerical parameters, title scale set to 1
        ths.change_box.Setup(ths.menu_strings[idx], units, ths.nvm.GetValue((NVM::Parameters)(idx + NVM::MPG_METRIC_FEED_1)), 1, 10000, precision, 1u);
        // Set AppTask
        ths.change_box.SetCallback(AppTask::GetCurrent());
        // Save axis index as ID
        ths.change_box.SetId(idx);
        // Show change box
        ths.change_box.Show(10000u);
      }
    }
    // Probe tab
    else if(ths.tabs.GetSelectedTab() == PROBE_TAB)
    {
      // Convert menu index to NVM index
      uint32_t nvm_idx = idx + NVM::PROBE_SEARCH_FEED;
      // Units and precision variables
      const char* units = nullptr;
      uint32_t precision = 0;

      if((nvm_idx == NVM::PROBE_BALL_TIP) || (nvm_idx == NVM::PROBE_POS_DEVIATION))
      {
        units = ths.grbl_comm.GetReportUnits();
        precision = ths.grbl_comm.GetReportUnitsPrecision();
      }
      else if((nvm_idx == NVM::PROBE_SEARCH_FEED) || (nvm_idx == NVM::PROBE_LOCK_FEED))
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
        ths.change_box.Setup(ths.menu_strings[idx], units, ths.grbl_comm.ConvertMetricToUnits(ths.nvm.GetValue((NVM::Parameters)(idx + NVM::PROBE_SEARCH_FEED))) , 1, 10000, precision, 1u);
        // Set AppTask
        ths.change_box.SetCallback(AppTask::GetCurrent());
        // Save axis index as ID
        ths.change_box.SetId(idx);
        // Show change box
        ths.change_box.Show(10000u);
      }
    }
    else
    {
      ; // Do nothing - MISRA rule
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
// ***   Private: ProcessButtonCallback function   *****************************
// *****************************************************************************
Result SettingsScr::ProcessButtonCallback(SettingsScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    SettingsScr& ths = *obj_ptr;
    // Get pressed button
    InputDrv::ButtonCallbackData btn = *((InputDrv::ButtonCallbackData*)ptr);

    // Change tab on button release only
    if((ths.tabs.IsEnabled()) && (btn.state == false))
    {
      // Buttons to switch tabs
      if(btn.btn == InputDrv::BTN_LEFT_DOWN)
      {
        ths.tabs.SetSelectedTab(ths.tabs.GetSelectedTab() - 1u);
      }
      else if(btn.btn == InputDrv::BTN_RIGHT_DOWN)
      {
        ths.tabs.SetSelectedTab(ths.tabs.GetSelectedTab() + 1u);
      }
      else
      {
        ; // Do nothing - MISRA rule
      }

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
void SettingsScr::UpdateStrings(void)
{
  char tmp_str[16u] = {0};
  // Count for menu
  uint32_t cnt = 0;
  // General tab
  if(tabs.GetSelectedTab() == GENERAL_TAB)
  {
    // ORDER OF STRINGS IN THIS ARRAY MUST EXACT MATCHED TO NVM::Parameters
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::TX_CONTROL], (nvm.GetCtrlTx() == GrblComm::CTRL_GPIO_PIN) ? "dedicated pin" : (nvm.GetCtrlTx() == GrblComm::CTRL_SW_COMMAND) ? "sw command" : (nvm.GetCtrlTx() == GrblComm::CTRL_PIN_AND_SW_CMD) ? "pin & sw cmd": "full control");
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::SCREEN_INVERT], nvm.GetValue(NVM::SCREEN_INVERT) ? "inverted" : "normal");
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::AUTO_MPG_ON_START], nvm.GetValue(NVM::AUTO_MPG_ON_START) ? "enabled" : "disabled");
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::SAVE_SCRIPT_RESULT], nvm.GetValue(NVM::SAVE_SCRIPT_RESULT) ? "enabled" : "disabled");
  }
  // MPG tab
  else if(tabs.GetSelectedTab() == MPG_TAB)
  {
    // ORDER OF STRINGS IN THIS ARRAY MUST EXACT MATCHED TO NVM::Parameters
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_METRIC_FEED_1], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), nvm.GetValue(NVM::MPG_METRIC_FEED_1), grbl_comm.GetUnitsScaler(GrblComm::MEASUREMENT_SYSTEM_METRIC), grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_METRIC)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_METRIC_FEED_2], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), nvm.GetValue(NVM::MPG_METRIC_FEED_2), grbl_comm.GetUnitsScaler(GrblComm::MEASUREMENT_SYSTEM_METRIC), grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_METRIC)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_METRIC_FEED_3], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), nvm.GetValue(NVM::MPG_METRIC_FEED_3), grbl_comm.GetUnitsScaler(GrblComm::MEASUREMENT_SYSTEM_METRIC), grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_METRIC)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_METRIC_FEED_4], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), nvm.GetValue(NVM::MPG_METRIC_FEED_4), grbl_comm.GetUnitsScaler(GrblComm::MEASUREMENT_SYSTEM_METRIC), grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_METRIC)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_IMPERIAL_FEED_1], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), nvm.GetValue(NVM::MPG_IMPERIAL_FEED_1), grbl_comm.GetUnitsScaler(GrblComm::MEASUREMENT_SYSTEM_IMPERIAL), grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_IMPERIAL)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_IMPERIAL_FEED_2], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), nvm.GetValue(NVM::MPG_IMPERIAL_FEED_2), grbl_comm.GetUnitsScaler(GrblComm::MEASUREMENT_SYSTEM_IMPERIAL), grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_IMPERIAL)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_IMPERIAL_FEED_3], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), nvm.GetValue(NVM::MPG_IMPERIAL_FEED_3), grbl_comm.GetUnitsScaler(GrblComm::MEASUREMENT_SYSTEM_IMPERIAL), grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_IMPERIAL)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_IMPERIAL_FEED_4], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), nvm.GetValue(NVM::MPG_IMPERIAL_FEED_4), grbl_comm.GetUnitsScaler(GrblComm::MEASUREMENT_SYSTEM_IMPERIAL), grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_IMPERIAL)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_ROTARY_FEED_1], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), nvm.GetValue(NVM::MPG_ROTARY_FEED_1), grbl_comm.GetUnitsScaler(GrblComm::MEASUREMENT_SYSTEM_ROTARY), grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_ROTARY)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_ROTARY_FEED_2], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), nvm.GetValue(NVM::MPG_ROTARY_FEED_2), grbl_comm.GetUnitsScaler(GrblComm::MEASUREMENT_SYSTEM_ROTARY), grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_ROTARY)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_ROTARY_FEED_3], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), nvm.GetValue(NVM::MPG_ROTARY_FEED_3), grbl_comm.GetUnitsScaler(GrblComm::MEASUREMENT_SYSTEM_ROTARY), grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_ROTARY)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_ROTARY_FEED_4], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), nvm.GetValue(NVM::MPG_ROTARY_FEED_4), grbl_comm.GetUnitsScaler(GrblComm::MEASUREMENT_SYSTEM_ROTARY), grbl_comm.GetUnits(GrblComm::MEASUREMENT_SYSTEM_ROTARY)));
  }
  // Probe tab
  else if(tabs.GetSelectedTab() == PROBE_TAB)
  {
    // ORDER OF STRINGS IN THIS ARRAY MUST EXACT MATCHED TO NVM::Parameters
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::PROBE_SEARCH_FEED],   grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), grbl_comm.ConvertMetricToUnits(nvm.GetValue(NVM::PROBE_SEARCH_FEED)), grbl_comm.GetReportSpeedScaler(), grbl_comm.GetReportSpeedUnits()));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::PROBE_LOCK_FEED],     grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), grbl_comm.ConvertMetricToUnits(nvm.GetValue(NVM::PROBE_LOCK_FEED)), grbl_comm.GetReportSpeedScaler(), grbl_comm.GetReportSpeedUnits()));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::PROBE_POS_DEVIATION], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), grbl_comm.ConvertMetricToUnits(nvm.GetValue(NVM::PROBE_POS_DEVIATION)), grbl_comm.GetReportUnitsScaler(), grbl_comm.GetReportUnits()));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::PROBE_BALL_TIP],      grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), grbl_comm.ConvertMetricToUnits(nvm.GetValue(NVM::PROBE_BALL_TIP)), grbl_comm.GetReportUnitsScaler(), grbl_comm.GetReportUnits()));
  }
  else
  {
    ; // Do nothing - MISRA rule
  }
  // Set menu count to match numbers of items on the tab
  menu.SetCount(cnt);
}

// ******************************************************************************
// ***   Private constructor   **************************************************
// ******************************************************************************
SettingsScr::SettingsScr() : menu(menu_items, NumberOf(menu_items)), change_box(Application::GetInstance().GetChangeValueBox()) {};
