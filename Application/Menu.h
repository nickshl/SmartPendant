//******************************************************************************
//  @file Menu.h
//  @author Nicolai Shlapunov
//
//  @details Menu: User Menu Class, header
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

#ifndef Menu_h
#define Menu_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"
#include "UiEngine.h"

#include "InputDrv.h"

// *****************************************************************************
// ***   Menu Class   **********************************************************
// *****************************************************************************
class Menu //: public VisObject
{
  public:
    // *************************************************************************
    // ***   Menu Item description structure   *********************************
    // *************************************************************************
    typedef struct typeMenuItem
    {
        char* text = nullptr;       // Pointer to string itself
        uint32_t n = 0u;            // Size of string
        String str;                 // String visual object
    } MenuItem;

    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    Menu() {};

    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    Menu(MenuItem* in_ptr, int32_t in_cnt) {ptr = in_ptr; cnt = in_cnt;}

    // *************************************************************************
    // ***   Public: Setup   ***************************************************
    // *************************************************************************
    Result Setup(MenuItem* in_ptr, int32_t in_cnt, int32_t x, int32_t y, int32_t w, int32_t h);

    // *************************************************************************
    // ***   Public: Setup   ***************************************************
    // *************************************************************************
    Result Setup(int32_t x, int32_t y, int32_t w, int32_t h);

    // *************************************************************************
    // ***   Public: SetCount   ************************************************
    // *************************************************************************
    void SetCount(int32_t in_cnt) {cnt = in_cnt; if(cur_pos >= cnt) cur_pos = 0;}

    // *************************************************************************
    // ***   Public: SetCallback   *********************************************
    // *************************************************************************
    void SetCallback(AppTask* task_in, void* param_in, CallbackPtr func_ent_in, CallbackPtr func_esc_in);

    // *************************************************************************
    // ***   Public: GetCurrentPos   *******************************************
    // *************************************************************************
    int32_t GetCurrentPos() {return cur_pos;}

    // *************************************************************************
    // ***   Public: CreateString   ********************************************
    // *************************************************************************
    Result CreateString(MenuItem& item, const char* str1, const char* str2);

    // *************************************************************************
    // ***   Public: CreateString   ********************************************
    // *************************************************************************
    Result CreateString(int32_t item, const char* str1, const char* str2);

    // *************************************************************************
    // ***   Public: Show   ****************************************************
    // *************************************************************************
    Result Show(uint32_t z = 0u);

    // *************************************************************************
    // ***   Public: Hide   ****************************************************
    // *************************************************************************
    Result Hide();

  private:
    static const uint8_t BORDER_W = 4u;

    // Callback data
    AppTask* task = nullptr; // Pointer to AppTask for all callbacks
    void* param = nullptr;   // Callback parameter
    CallbackPtr func_ent = nullptr; // Pointer callback function after press enter
    CallbackPtr func_esc = nullptr; // Pointer callback function after press back

    // List that contains all menu elements
    VisList list;

    MenuItem* ptr = nullptr;
    // Count of menu items
    int32_t cnt = 0;

    // Selection box
    Box box;
    // Current menu position
    int32_t cur_pos = 0;

    // Soft Buttons
    UiButton left_btn;
    UiButton right_btn;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();

    // Encoder callback entry
    InputDrv::CallbackListEntry enc_cble;
    // Button callback entry
    InputDrv::CallbackListEntry btn_cble;

    // *************************************************************************
    // ***   Private: ProcessEncoderCallback function   ************************
    // *************************************************************************
    static Result ProcessEncoderCallback(Menu* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(Menu* obj_ptr, void* ptr);
};

#endif
