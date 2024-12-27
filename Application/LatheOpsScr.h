//******************************************************************************
//  @file LatheOpsScr.h
//  @author Nicolai Shlapunov
//
//  @details LatheOpsScr: User LatheOpsScr Class, header
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

#ifndef LatheOpsScr_h
#define LatheOpsScr_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"
#include "UiEngine.h"

#include "IScreen.h"
#include "DataWindow.h"
#include "GrblComm.h"
#include "InputDrv.h"
#include "Tabs.h"
#include "Menu.h"
#include "ChangeValueBox.h"

// *****************************************************************************
// ***   Local const variables   ***********************************************
// *****************************************************************************

// *****************************************************************************
// ***   Constants   ***********************************************************
// *****************************************************************************
// Should be set to maximum possible menu items!
constexpr uint32_t MAX_MENU_ITEMS = 10u;

// *****************************************************************************
// *****************************************************************************
// ***   LatheOpsScr Class   ***************************************************
// *****************************************************************************
// *****************************************************************************
class LatheOpsScr : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static LatheOpsScr& GetInstance();

    // *************************************************************************
    // ***   Public: GetMenu function   ****************************************
    // *************************************************************************
    Menu& GetMenu() {return(menu);}

    // *************************************************************************
    // ***   Public: Setup function   ******************************************
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
    static constexpr uint8_t BORDER_W = 4u;

    // Pages
    Tabs tabs;

    // Pointers to screens
    IScreen* tab[16u] = {0};
    // Screen counter
    uint32_t tabs_cnt = 0u;
    // Current screen index
    uint32_t tab_idx = 0u;

    // Strings
    char str[MAX_MENU_ITEMS][32u + 1u] = {0};
    // Menu items
    Menu::MenuItem menu_items[MAX_MENU_ITEMS];
    // Menu object
    Menu menu;

    // Soft Button
    UiButton& right_btn;

    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // Button callback entry
    InputDrv::CallbackListEntry btn_cble;

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(LatheOpsScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ChangeTab function   *************************************
    // *************************************************************************
    void ChangeTab(uint8_t tabn);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    LatheOpsScr();
};

// *****************************************************************************
// *****************************************************************************
// ***   TurnGeneratorTab Class   **********************************************
// *****************************************************************************
// *****************************************************************************
class TurnGeneratorTab : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static TurnGeneratorTab& GetInstance();

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
    // Enum with menu item indexes
    enum
    {
      TURN_LENGTH,
      TURN_DIAMETER,
      TURN_STEP,
      TURN_FEED,
      TURN_SPEED,
      TURN_FINE_STEP,
      TURN_FINE_FEED,
      TURN_FINE_SPEED,
      TURN_GENERATE,
      TURN_MAX_ITEMS
    };

    // Check that it will fit
    static_assert(TURN_MAX_ITEMS < MAX_MENU_ITEMS, "Not enough menu items!");

    // Menu item strings. This array must match the enum above!
    const char* const menu_strings[TURN_MAX_ITEMS] =
    {
      "Turn Length", "Turn Diameter", "Rough Step", "Rough Feed", "Rough Speed", "Finish Step", "Finish Feed", "Finish Speed", "Generate"
    };

    // Array with values
    int32_t values[TURN_MAX_ITEMS] = {0};

    // Menu object
    Menu& menu;

    // Object to change numerical parameters
    ChangeValueBox& change_box;

    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // *************************************************************************
    // ***   Private: ProcessMenuCallback function   ***************************
    // *************************************************************************
    static Result ProcessMenuCallback(TurnGeneratorTab* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: UpdateStrings function   *********************************
    // *************************************************************************
    void UpdateStrings();

    // *************************************************************************
    // ***   Private: GenerateGcode function   *********************************
    // *************************************************************************
    Result GenerateGcode();

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    TurnGeneratorTab();
};

// *****************************************************************************
// *****************************************************************************
// ***   FaceGeneratorTab Class   **********************************************
// *****************************************************************************
// *****************************************************************************
class FaceGeneratorTab : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static FaceGeneratorTab& GetInstance();

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
    // Enum with menu item indexes
    enum
    {
      FACE_LENGTH,
      FACE_DIAMETER,
      FACE_STEP,
      FACE_FEED,
      FACE_SPEED,
      FACE_FINE_STEP,
      FACE_FINE_FEED,
      FACE_FINE_SPEED,
      FACE_GENERATE,
      FACE_MAX_ITEMS
    };

    // Check that it will fit
    static_assert(FACE_MAX_ITEMS < MAX_MENU_ITEMS, "Not enough menu items!");

    // Menu item strings. This array must match the enum above!
    const char* const menu_strings[FACE_MAX_ITEMS] =
    {
      "Face Length", "Face Diameter", "Rough Step", "Rough Feed", "Rough Speed", "Finish Step", "Finish Feed", "Finish Speed", "Generate"
    };

    // Array with values
    int32_t values[FACE_MAX_ITEMS] = {0};

    // Menu object
    Menu& menu;

    // Object to change numerical parameters
    ChangeValueBox& change_box;

    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // *************************************************************************
    // ***   Private: ProcessMenuCallback function   ***************************
    // *************************************************************************
    static Result ProcessMenuCallback(FaceGeneratorTab* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: UpdateStrings function   *********************************
    // *************************************************************************
    void UpdateStrings();

    // *************************************************************************
    // ***   Private: GenerateGcode function   *********************************
    // *************************************************************************
    Result GenerateGcode();

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    FaceGeneratorTab();
};

// *****************************************************************************
// *****************************************************************************
// ***   CutGeneratorTab Class   ***********************************************
// *****************************************************************************
// *****************************************************************************
class CutGeneratorTab : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static CutGeneratorTab& GetInstance();

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
    // Enum with menu item indexes
    enum
    {
      CUT_DIAMETER,
      CUT_WIDTH,
      CUT_STEP,
      CUT_FEED,
      CUT_SPEED,
      CUT_GENERATE,
      CUT_MAX_ITEMS
    };

    // Check that it will fit
    static_assert(CUT_MAX_ITEMS < MAX_MENU_ITEMS, "Not enough menu items!");

    // Menu item strings. This array must match the enum above!
    const char* const menu_strings[CUT_MAX_ITEMS] =
    {
      "Cut Diameter", "Cut Width", "Cut Step", "Cut Feed", "Cut Speed", "Generate"
    };

    // Array with values
    int32_t values[CUT_MAX_ITEMS] = {0};

    // Menu object
    Menu& menu;

    // Object to change numerical parameters
    ChangeValueBox& change_box;

    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // *************************************************************************
    // ***   Private: ProcessMenuCallback function   ***************************
    // *************************************************************************
    static Result ProcessMenuCallback(CutGeneratorTab* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: UpdateStrings function   *********************************
    // *************************************************************************
    void UpdateStrings();

    // *************************************************************************
    // ***   Private: GenerateGcode function   *********************************
    // *************************************************************************
    Result GenerateGcode();

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    CutGeneratorTab();
};

#endif
