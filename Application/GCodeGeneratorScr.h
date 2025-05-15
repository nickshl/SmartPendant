//******************************************************************************
//  @file GCodeGeneratorScr.h
//  @author Nicolai Shlapunov
//
//  @details CodeGeneratorScr: User CodeGeneratorScr Class, header
//
//  @copyright Copyright (c) 2025, Devtronic & Nicolai Shlapunov
//             All rights reserved.
//
//  @section SUPPORT
//
//   Devtronic invests time and resources providing this open source code,
//   please support Devtronic and open-source hardware/software by
//   donations and/or purchasing products from Devtronic.
//
//******************************************************************************

#ifndef GCodeGeneratorScr_h
#define GCodeGeneratorScr_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"
#include "UiEngine.h"

#include "IScreen.h"
#include "Tabs.h"
#include "Menu.h"
#include "MsgBox.h"
#include "ChangeValueBox.h"

#include "Little-C.h"

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************
#define BG_Z (100)

// *****************************************************************************
// ***   CodeGeneratorScr Class   **********************************************
// *****************************************************************************
class GCodeGeneratorScr : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static GCodeGeneratorScr& GetInstance();

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
    static const uint8_t BORDER_W = 4u;

    // Pointer to text buffer used if program loaded completely
    char* p_text = nullptr;

    // Interpreter for C-like language to generate G-Code
    LittleC interpreter;

    // Pages
    Tabs tabs;
    // Caption
    char script_caption_str[32u];

    // Strings
    char str[32u][32u + 1u] = {0};
    // menu items
    Menu::MenuItem menu_items[32u];
    // Menu object
    Menu menu;

    // Message box to display errors
    MsgBox msg_box;

    // Object to change numerical parameters
    ChangeValueBox& change_box;
    // Caption for change_box
    char change_box_caption_str[32u];
    // Units for change_box
    char change_box_units_str[16u];

    // *************************************************************************
    // *************************************************************************
    // *************************************************************************

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // *************************************************************************
    // ***   Private: ProcessMenuOkCallback function   *************************
    // *************************************************************************
    static Result ProcessMenuOkCallback(GCodeGeneratorScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessMenuCancelCallback function   *********************
    // *************************************************************************
    static Result ProcessMenuCancelCallback(GCodeGeneratorScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: UpdateMenuStrings   **************************************
    // *************************************************************************
    void UpdateMenuStrings();

    // *************************************************************************
    // ***   Private: AllocateDataBuffer   *************************************
    // *************************************************************************
    char* AllocateDataBuffer(uint32_t size);

    // *************************************************************************
    // ***   Private: ReleaseDataPointer   *************************************
    // *************************************************************************
    void ReleaseDataPointer();

    // *************************************************************************
    // ***   Private: GetGlobalVariableCommentString   *************************
    // *************************************************************************
    bool GetGlobalVariableCommentString(uint32_t variable_idx, char* ptr, uint32_t n, uint32_t pos);

    // *************************************************************************
    // ***   Private: GetGlobalVariableCommentNumber   *************************
    // *************************************************************************
    bool GetGlobalVariableCommentNumber(uint32_t variable_idx, int32_t& value, uint32_t pos);

    // *************************************************************************
    // ***   Public: GetGlobalVariableDescription   ****************************
    // *************************************************************************
    bool GetGlobalVariableDescription(uint32_t variable_idx, char* ptr, uint32_t n);

    // *************************************************************************
    // ***   Public: GetGlobalVariableScaler   *********************************
    // *************************************************************************
    bool GetGlobalVariableScaler(uint32_t variable_idx, int32_t& scaler);

    // *************************************************************************
    // ***   Public: GetGlobalVariableUnits   **********************************
    // *************************************************************************
    bool GetGlobalVariableUnits(uint32_t variable_idx, char* ptr, uint32_t n);

    // *************************************************************************
    // ***   Private: GetGlobalVariableMinVal   ********************************
    // *************************************************************************
    bool GetGlobalVariableMinVal(uint32_t variable_idx, int32_t& min);

    // *************************************************************************
    // ***   Private: GetGlobalVariableMaxVal   ********************************
    // *************************************************************************
    bool GetGlobalVariableMaxVal(uint32_t variable_idx, int32_t& max);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    GCodeGeneratorScr();
};

#endif
