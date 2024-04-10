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

#include "GrblComm.h"

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
  // Always good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Private: ProcessButtonCallback function   *****************************
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

    if((uint32_t)ptr == TX_CONTROL)
    {
      uint8_t val = ths.nvm.GetCtrlTx() + 1u;    // Get current value and increment it by 1
      if(val >= GrblComm::CTRL_TX_CNT) val = 0u; // Check overflow
      ths.nvm.SetCtrlTx(val);                    // Store new value
    }
    else if((uint32_t)ptr == MODE)
    {
      ths.nvm.SetMode(!ths.nvm.GetMode());
    }
    else if((uint32_t)ptr == UNITS)
    {
      ths.nvm.SetUnits(!ths.nvm.GetUnits());
    }
    else if((uint32_t)ptr == SCREEN_INVERT)
    {
      ths.nvm.SetDisplayInvert(!ths.nvm.GetDisplayInvert());
      ths.display_drv.InvertDisplay(ths.nvm.GetDisplayInvert());
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
// ***   Private: Update function   ********************************************
// *****************************************************************************
void SettingsScr::UpdateStrings(void)
{
  menu.CreateString(menu_items[TX_CONTROL], "MPG request", (nvm.GetCtrlTx() == GrblComm::CTRL_GPIO_PIN) ? "dedicated pin" : (nvm.GetCtrlTx() == GrblComm::CTRL_SW_COMMAND) ? "sw command": "full control");
  menu.CreateString(menu_items[MODE], "Mode", nvm.GetMode() ? "lathe" : "mill");
  menu.CreateString(menu_items[UNITS], "Units", nvm.GetUnits() ? "metric" : "imperial");
  menu.CreateString(menu_items[SCREEN_INVERT], "Display Inversion", nvm.GetDisplayInvert() ? "inverted" : "normal");
}
