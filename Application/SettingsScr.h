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
    // Enum with menu items
    // TODO: THIS MUST EXACT MATCHED TO NVM::Parameters - need to be fixed
    enum
    {
      TX_CONTROL,
      SCREEN_INVERT,
      PROBE_SEARCH_FEED,
      PROBE_LOCK_FEED,
      PROBE_BALL_TIP,
      JOGGING_SPEED,
      MAX_ITEMS
    };

    // Menu item strings. This array must match the enum above!
    const char* const menu_strings[MAX_ITEMS] =
    {
      "MPG request", "Display Inversion", "Probe search speed", "Probe lock speed", "Probe ball tip",
      "Jogging speed"
    };

    // Strings
    char str[MAX_ITEMS][32u + 1u] = {0};
    // menu items
    Menu::MenuItem menu_items[MAX_ITEMS];
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

    // *************************************************************************
    // ***   Private: ProcessMenuCallback function   ***************************
    // *************************************************************************
    static Result ProcessMenuCallback(SettingsScr* obj_ptr, void* ptr);

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
