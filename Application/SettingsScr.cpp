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
#include "FramedUart.h"

#include "Application.h"

// Framed UART transport layer
extern FramedUart framed_uart;
// Transport actually in use
extern bool framed_transport_in_use;

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
  tabs.SetParams(0, y, DisplayDrv::GetInstance().GetScreenW(), 40, 4u);
  // General settings tab
  tabs.SetText(0u, "GENERAL", nullptr, Font_10x18::GetInstance());
  // MPG settings tab
  tabs.SetText(1u, "MPG", nullptr, Font_10x18::GetInstance());
  // Probe settings tab
  tabs.SetText(2u, "PROBE", nullptr, Font_10x18::GetInstance());
  // Link information tab
  tabs.SetText(3u, "INFO", nullptr, Font_10x18::GetInstance());
  // Set callback
  tabs.SetCallback(AppTask::GetCurrent());

  // Fill menu_items
  for(uint32_t i = 0u; i < NumberOf(menu_items); i++)
  {
    menu_items[i].text = str[i];
    menu_items[i].n = sizeof(str[i]);
  }
  // Set callback
  menu.SetCallback(AppTask::GetCurrent(), this, reinterpret_cast<CallbackPtr>(ProcessMenuCallback), reinterpret_cast<CallbackPtr>(ProcessMenuCancelCallback));
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

  // In case if it shown, we should hide it. Otherwise a stale box callback
  // would be routed to the next screen and misinterpreted there.
  change_box.Hide();

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
  // Info tab shows counters that change by themselves, so it has to be
  // refreshed. Other tabs show settings and are updated when they change.
  if((tabs.GetSelectedTab() == INFO_TAB) && RtosTick::CheckTimeDifferenceMs(update_time_ms, 1000u))
  {
    // Update strings
    UpdateStrings();
    // Update last timestamp
    update_time_ms = RtosTick::GetTimeMs(); // Update every 1 second
  }

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
        // Convert menu index to NVM index(first item on the tab is MPG_MATCH_SPEED_LIMITS)
        // and save value as is since we have separate values for metric and imperial
        nvm.SetValue((NVM::Parameters)(change_box.GetId() + NVM::MPG_MATCH_SPEED_LIMITS), change_box.GetValue());
      }
      // General tab - the numeric link parameters are edited this way. The
      // id carried by the box is the menu index, which maps to the NVM
      // index by the same fixed offset the menu callback uses.
      else if(tabs.GetSelectedTab() == GENERAL_TAB)
      {
        nvm.SetValue((NVM::Parameters)(change_box.GetId() + NVM::TX_CONTROL), change_box.GetValue());
        // Applied at once - only port, baud and transport need a reboot
        framed_uart.SetAttempts(nvm.GetValue(NVM::FRAME_ATTEMPTS));
        framed_uart.SetAckTimeoutFloor(nvm.GetValue(NVM::ACK_MIN_MS));
      }
      // Probe tab
      else if(tabs.GetSelectedTab() == PROBE_TAB)
      {
        // Convert menu index to NVM index
        uint32_t nvm_idx = change_box.GetId() + NVM::PROBE_SEARCH_FEED;
        // Check if edited parameter is one of the probe feeds
        if((nvm_idx == NVM::PROBE_SEARCH_FEED) || (nvm_idx == NVM::PROBE_LOCK_FEED))
        {
          // Value edited as whole mm/min in metric and as inches/min * 100 in imperial
          int32_t feed = grbl_comm.IsMetric() ? change_box.GetValue() : (int32_t)grbl_comm.ConvertUnitsFeedX100ToMetric(change_box.GetValue());
          // Feed is always stored as whole mm/min, zero feed is invalid
          if(feed < 1) feed = 1;
          // Save value
          nvm.SetValue((NVM::Parameters)(nvm_idx), feed);
        }
        else
        {
          // Save value - probe parameters always saved as metric
          nvm.SetValue((NVM::Parameters)(nvm_idx), grbl_comm.ConvertUnitsToMetric(change_box.GetValue()));
        }
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
  else if(ptr == &Application::GetInstance().GetMsgBox())
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
      else if(nvm_idx == NVM::BAUD_RATE)
      {
        // Get current baud rate
        int32_t baud = ths.nvm.GetValue(NVM::BAUD_RATE);
        // Find next baud rate
        if(baud < 19200) baud = 19200;
        else if(baud < 38400) baud = 38400;
        else if(baud < 57600) baud = 57600;
        else if(baud < 115200) baud = 115200;
        else if(baud < 230400) baud = 230400;
        else baud = 9600;
        // Save new baud rate
        ths.nvm.SetValue(NVM::BAUD_RATE, baud);
      }
      else if(nvm_idx == NVM::TRANSPORT)
      {
        ths.nvm.SetValue(NVM::TRANSPORT, !ths.nvm.GetValue(NVM::TRANSPORT));
      }
      else if(nvm_idx == NVM::FRAME_ATTEMPTS)
      {
        // Transmissions of one frame before the link is declared down.
        // Same range as the controller setting it has to match.
        // Setup object to change numerical parameters, no units, no fraction
        ths.change_box.Setup(ths.menu_strings[nvm_idx], nullptr, ths.nvm.GetValue(NVM::FRAME_ATTEMPTS), FramedUart::MIN_ATTEMPTS, FramedUart::MAX_ATTEMPTS, 0u, 1u);
        // Set AppTask
        ths.change_box.SetCallback(AppTask::GetCurrent());
        // Save menu index as ID
        ths.change_box.SetId(idx);
        // Show change box
        ths.change_box.Show(10000u);
      }
      else if(nvm_idx == NVM::ACK_MIN_MS)
      {
        // Floor under the baud derived acknowledge timeout. Zero means the
        // derived value is used as is - only raise it for a link with
        // latency the baud rate does not describe, such as a wireless
        // bridge. Same range as the controller setting it pairs with.
        // Setup object to change numerical parameters, no fraction
        ths.change_box.Setup(ths.menu_strings[nvm_idx], "ms", ths.nvm.GetValue(NVM::ACK_MIN_MS), 0, FramedUart::MAX_ACK_MIN_MS, 0u, 1u);
        // Set AppTask
        ths.change_box.SetCallback(AppTask::GetCurrent());
        // Save menu index as ID
        ths.change_box.SetId(idx);
        // Show change box
        ths.change_box.Show(10000u);
      }
      else if(nvm_idx == NVM::AUTO_MPG_ON_START)
      {
        ths.nvm.SetValue(NVM::AUTO_MPG_ON_START, !ths.nvm.GetValue(NVM::AUTO_MPG_ON_START));
      }
      else if(nvm_idx == NVM::SAVE_SCRIPT_RESULT)
      {
        ths.nvm.SetValue(NVM::SAVE_SCRIPT_RESULT, !ths.nvm.GetValue(NVM::SAVE_SCRIPT_RESULT));
      }
      else if(nvm_idx == NVM::SCREEN_INVERT)
      {
        ths.nvm.SetValue(NVM::SCREEN_INVERT, !ths.nvm.GetValue(NVM::SCREEN_INVERT));
        ths.display_drv.InvertDisplay(ths.nvm.GetValue(NVM::SCREEN_INVERT));
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
      uint32_t nvm_idx = idx + NVM::MPG_MATCH_SPEED_LIMITS;
      // Units and precision variables
      const char* units = nullptr;
      uint32_t precision = 0;

      if(nvm_idx == NVM::MPG_MATCH_SPEED_LIMITS)
      {
        // On/off option - toggle it, no change box needed
        ths.nvm.SetValue(NVM::MPG_MATCH_SPEED_LIMITS, !ths.nvm.GetValue(NVM::MPG_MATCH_SPEED_LIMITS));
      }
      else if((nvm_idx >= NVM::MPG_METRIC_FEED_1) && (nvm_idx <= NVM::MPG_METRIC_FEED_4))
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
        ths.change_box.Setup(ths.menu_strings[nvm_idx], units, ths.nvm.GetValue((NVM::Parameters)(nvm_idx)), 1, 10000, precision, 1u);
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
      // Value in display units
      int32_t value = 0;

      if((nvm_idx == NVM::PROBE_BALL_TIP) || (nvm_idx == NVM::PROBE_POS_DEVIATION))
      {
        units = ths.grbl_comm.GetReportUnits();
        precision = ths.grbl_comm.GetReportUnitsPrecision();
        value = ths.grbl_comm.ConvertMetricToUnits(ths.nvm.GetValue((NVM::Parameters)(nvm_idx)));
      }
      else if((nvm_idx == NVM::PROBE_SEARCH_FEED) || (nvm_idx == NVM::PROBE_LOCK_FEED))
      {
        units = ths.grbl_comm.GetReportFeedUnits();
        // Feed edited as whole mm/min in metric and as inches/min with two
        // decimal places in imperial(1 mm/min is only ~0.04 inches/min)
        precision = ths.grbl_comm.IsMetric() ? 0u : 2u;
        value = ths.grbl_comm.IsMetric() ? ths.nvm.GetValue((NVM::Parameters)(nvm_idx)) : (int32_t)ths.grbl_comm.ConvertMetricFeedToUnitsX100(ths.nvm.GetValue((NVM::Parameters)(nvm_idx)));
      }
      else
      {
        ; // Do nothing - MISRA rule
      }

      // Show change box only if index is found
      if(units != nullptr)
      {
        // Setup object to change numerical parameters, title scale set to 1
        ths.change_box.Setup(ths.menu_strings[nvm_idx], units, value, 1, 10000, precision, 1u);
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
// ***   Private: ProcessMenuCancelCallback function   *************************
// *****************************************************************************
Result SettingsScr::ProcessMenuCancelCallback(SettingsScr* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Because of "this" pointer we can't use non-static members function to
    // handle callback, so we have to provide pointer to object.
    SettingsScr& ths = *obj_ptr;
    // Convert pointer to index. Zero is a valid row, so a null pointer here
    // means the first one, not "nothing selected".
    uint32_t idx = (uint32_t)ptr;

    // Cancel on the Transport line of the Info tab clears the link
    // counters. They are free running and a reboot is the only other way to
    // zero them - and a reboot drops the link, so it can't be used to watch
    // a fresh count. The controller has $MPG=RESET for the same reason.
    if((ths.tabs.GetSelectedTab() == INFO_TAB) && (idx == INFO_TRANSPORT_ITEM))
    {
      framed_uart.ClearStats();
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
  // Buffer must fit the longest string: "393.70 inches/min" for feed
  // converted from 10000 mm/min maximum
  char tmp_str[20u] = {0};
  // Count for menu
  uint32_t cnt = 0;
  // General tab
  if(tabs.GetSelectedTab() == GENERAL_TAB)
  {
    // ORDER OF STRINGS IN THIS ARRAY MUST EXACT MATCHED TO NVM::Parameters
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::TX_CONTROL], (nvm.GetCtrlTx() == GrblComm::CTRL_GPIO_PIN) ? "dedicated pin" : (nvm.GetCtrlTx() == GrblComm::CTRL_SW_COMMAND) ? "sw command" : (nvm.GetCtrlTx() == GrblComm::CTRL_PIN_AND_SW_CMD) ? "pin & sw cmd": "full control");
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::BAUD_RATE], psnprintf(tmp_str, NumberOf(tmp_str), "%lu", nvm.GetValue(NVM::BAUD_RATE)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::TRANSPORT], nvm.GetValue(NVM::TRANSPORT) ? "Framed" : "Plain");
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::FRAME_ATTEMPTS], psnprintf(tmp_str, NumberOf(tmp_str), "%lu", nvm.GetValue(NVM::FRAME_ATTEMPTS)));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::ACK_MIN_MS], nvm.GetValue(NVM::ACK_MIN_MS) ? psnprintf(tmp_str, NumberOf(tmp_str), "%lu ms", nvm.GetValue(NVM::ACK_MIN_MS)) : "auto");
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::AUTO_MPG_ON_START], nvm.GetValue(NVM::AUTO_MPG_ON_START) ? "enabled" : "disabled");
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::SAVE_SCRIPT_RESULT], nvm.GetValue(NVM::SAVE_SCRIPT_RESULT) ? "enabled" : "disabled");
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::SCREEN_INVERT], nvm.GetValue(NVM::SCREEN_INVERT) ? "inverted" : "normal");
  }
  // MPG tab
  else if(tabs.GetSelectedTab() == MPG_TAB)
  {
    // ORDER OF STRINGS IN THIS ARRAY MUST EXACT MATCHED TO NVM::Parameters
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::MPG_MATCH_SPEED_LIMITS], nvm.GetValue(NVM::MPG_MATCH_SPEED_LIMITS) ? "on" : "off");
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
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::PROBE_SEARCH_FEED],   grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), grbl_comm.ConvertMetricFeedToUnitsX100(nvm.GetValue(NVM::PROBE_SEARCH_FEED)), 100, grbl_comm.GetReportFeedUnits(), true));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::PROBE_LOCK_FEED],     grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), grbl_comm.ConvertMetricFeedToUnitsX100(nvm.GetValue(NVM::PROBE_LOCK_FEED)), 100, grbl_comm.GetReportFeedUnits(), true));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::PROBE_POS_DEVIATION], grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), grbl_comm.ConvertMetricToUnits(nvm.GetValue(NVM::PROBE_POS_DEVIATION)), grbl_comm.GetReportUnitsScaler(), grbl_comm.GetReportUnits()));
    menu.CreateString(menu_items[cnt++], menu_strings[NVM::PROBE_BALL_TIP],      grbl_comm.ValueToStringWithScalerAndUnits(tmp_str, NumberOf(tmp_str), grbl_comm.ConvertMetricToUnits(nvm.GetValue(NVM::PROBE_BALL_TIP)), grbl_comm.GetReportUnitsScaler(), grbl_comm.GetReportUnits()));
  }
  // Info tab
  else if(tabs.GetSelectedTab() == INFO_TAB)
  {
    // Transport in use, not the one in settings - the counters below belong
    // to it, and it changes on reboot only
    menu.CreateString(menu_items[cnt++], "Transport", framed_transport_in_use ? "Framed" : "Plain");

    if(framed_transport_in_use)
    {
      const FramedUart::stats_t& stats = framed_uart.GetStats();

      // Link status
      menu.CreateString(menu_items[cnt++], "Link", framed_uart.IsLinkUp() ? "up" : "down");
      // What we sent and what came back for it
      menu.CreateString(menu_items[cnt++], "Data frames sent",      psnprintf(tmp_str, NumberOf(tmp_str), "%lu", stats.tx_data));
      menu.CreateString(menu_items[cnt++], "Data retransmits",      psnprintf(tmp_str, NumberOf(tmp_str), "%lu", stats.tx_retry));
      menu.CreateString(menu_items[cnt++], "Ack received",          psnprintf(tmp_str, NumberOf(tmp_str), "%lu", stats.rx_ack));
      menu.CreateString(menu_items[cnt++], "Nak received",          psnprintf(tmp_str, NumberOf(tmp_str), "%lu", stats.rx_nak));
      // What the controller sent and what we answered to it
      menu.CreateString(menu_items[cnt++], "Data frames received",  psnprintf(tmp_str, NumberOf(tmp_str), "%lu", stats.rx_data));
      menu.CreateString(menu_items[cnt++], "Duplicates received",   psnprintf(tmp_str, NumberOf(tmp_str), "%lu", stats.rx_dup));
      menu.CreateString(menu_items[cnt++], "Ack sent",              psnprintf(tmp_str, NumberOf(tmp_str), "%lu", stats.tx_ack));
      menu.CreateString(menu_items[cnt++], "Nak sent",              psnprintf(tmp_str, NumberOf(tmp_str), "%lu", stats.tx_nak));
      // Faults
      menu.CreateString(menu_items[cnt++], "CRC errors",            psnprintf(tmp_str, NumberOf(tmp_str), "%lu", stats.rx_crc_err));
      menu.CreateString(menu_items[cnt++], "Buffer full",           psnprintf(tmp_str, NumberOf(tmp_str), "%lu", stats.rx_no_room));
      menu.CreateString(menu_items[cnt++], "Transmit drops",        psnprintf(tmp_str, NumberOf(tmp_str), "%lu", stats.tx_dropped));
      menu.CreateString(menu_items[cnt++], "Link losses",           psnprintf(tmp_str, NumberOf(tmp_str), "%lu", stats.link_downs));
    }
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
