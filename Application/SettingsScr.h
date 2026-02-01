//******************************************************************************
//  @file SettingsScr.h
//  @author Nicolai Shlapunov
//
//  @details SettingsScr: User SettingsScr Class, header
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

#ifndef SettingsScr_h
#define SettingsScr_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"
#include "UiEngine.h"

#include "IScreen.h"
#include "Tabs.h"
#include "InputDrv.h"
#include "NVM.h"
#include "Menu.h"
#include "ChangeValueBox.h"

// *****************************************************************************
// ***   Local const variables   ***********************************************
// *****************************************************************************

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************
#define BG_Z (100)

// *****************************************************************************
// ***   SettingsScr Class   ***************************************************
// *****************************************************************************
class SettingsScr : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static SettingsScr& GetInstance();

    // *************************************************************************
    // ***   Setup function   **************************************************
    // *************************************************************************
    virtual Result Setup(int32_t y, int32_t height);

    // *************************************************************************
    // ***   Public: Show   ****************************************************
    // *************************************************************************
    virtual Result Show();

    // *************************************************************************
    // ***   Public: Hide   ****************************************************
    // *************************************************************************
    virtual Result Hide();

    // *************************************************************************
    // ***   Public: TimerExpired   ********************************************
    // *************************************************************************
    virtual Result TimerExpired(uint32_t interval);

    // *************************************************************************
    // ***   Public: ProcessCallback   *****************************************
    // *************************************************************************
    virtual Result ProcessCallback(const void* ptr);

  private:
    // Number of menu strings. Set to max number on tab.
    static constexpr uint32_t MENU_ITEMS = 12u;

    // Enum with menu items
    enum
    {
      GENERAL_TAB,
      MPG_TAB,
      PROBE_TAB,
      MAX_TABS
    };

    // ORDER OF STRINGS IN THIS ARRAY MUST EXACT MATCHED TO NVM::Parameters
    const char* const menu_strings[NVM::MAX_VALUES] =
    {
      "Version",
      // General
      "MPG request", "Display Inversion", "Auto MPG on startup",
      // MPG
      "Metric Feed 1", "Metric Feed 2", "Metric Feed 3", "Metric Feed 4",
      "Imperial Feed 1", "Imperial Feed 2", "Imperial Feed 3", "Imperial Feed 4",
      "Rotary Feed 1", "Rotary Feed 2", "Rotary Feed 3", "Rotary Feed 4",
      // Probe
      "Probe search speed", "Probe lock speed", "Probe ball tip"
    };

    // Pages
    Tabs tabs;

    // Strings
    char str[MENU_ITEMS][32u + 1u] = {0};
    // menu items
    Menu::MenuItem menu_items[MENU_ITEMS];
    // Menu object
    Menu menu;

    // Object to change numerical parameters
    ChangeValueBox& change_box;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();
    // NVM instance
    NVM& nvm = NVM::GetInstance();

    // Button callback entry
    InputDrv::CallbackListEntry btn_cble;

    // *************************************************************************
    // ***   Private: ProcessMenuCallback function   ***************************
    // *************************************************************************
    static Result ProcessMenuCallback(SettingsScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(SettingsScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: UpdateStrings function   *********************************
    // *************************************************************************
    void UpdateStrings();

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    SettingsScr();
};

#endif
