//******************************************************************************
//  @file MsgBox.h
//  @author Nicolai Shlapunov
//
//  @details MsgBox: User MsgBox Class, header
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

#ifndef MsgBox_h
#define MsgBox_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"
#include "UiEngine.h"

#include "InputDrv.h"

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************
#define BG_Z (100)

// *****************************************************************************
// ***   MsgBox Class   ********************************************************
// *****************************************************************************
class MsgBox
{
  public:
    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    MsgBox() {};

    // *************************************************************************
    // ***   Public: Setup function   ******************************************
    // *************************************************************************
    Result Setup(const char* title, const char* text, uint8_t title_scale = 2u);

    // *************************************************************************
    // ***   Set callback function   *******************************************
    // *************************************************************************
    void SetCallback(AppTask* task, CallbackPtr func = nullptr, void* param = nullptr);

    // *************************************************************************
    // ***   Public: Show   ****************************************************
    // *************************************************************************
    Result Show(uint32_t z);

    // *************************************************************************
    // ***   Public: Hide   ****************************************************
    // *************************************************************************
    Result Hide();

    // *************************************************************************
    // ***   Public: IsShow   **************************************************
    // *************************************************************************
    bool IsShow() {return list.IsShow();}

    // *************************************************************************
    // ***   Public: GetValue   ************************************************
    // *************************************************************************
    Result GetResult() {return selection;}

  private:
    static const uint8_t BORDER_W = 4u;

    // Callback function pointer
    AppTask* callback_task = nullptr;
    CallbackPtr callback_func = nullptr;
    void* callback_param = nullptr;

    // Result variable
    Result selection = Result::ERR_CANCEL;

    // List that contains all menu elements
    VisList list;

    // Box around
    Box box;
    // Box around
    ShadowBox shadowbox;

    // Strings for caption and text
    String msg_box_caption;
    MultiLineString msg_box_text;

    // Soft Buttons
    UiButton left_btn;
    UiButton right_btn;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();

    // Button callback entry
    InputDrv::CallbackListEntry btn_cble;

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(MsgBox* obj_ptr, void* ptr);
};

#endif
