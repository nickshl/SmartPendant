//******************************************************************************
//  @file ChangeValueBox.h
//  @author Nicolai Shlapunov
//
//  @details ChangeValueBox: User ChangeValueBox Class, header
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

#ifndef ChangeValueBox_h
#define ChangeValueBox_h

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

// *****************************************************************************
// ***   Local const variables   ***********************************************
// *****************************************************************************

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************
#define BG_Z (100)

// *****************************************************************************
// ***   ChangeValueBox Class   ************************************************
// *****************************************************************************
class ChangeValueBox
{
  public:
    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    ChangeValueBox() {};

    // *************************************************************************
    // ***   Public: GetValue   ************************************************
    // *************************************************************************
    int32_t GetValue() {return value_dw.GetNumber();}

    // *************************************************************************
    // ***   Public: Setup function   ******************************************
    // *************************************************************************
    Result Setup(const char* title, const char* units, int32_t val, int32_t min, int32_t max, uint32_t point_pos, uint8_t title_scale = 2u);

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
    // ***   Public: SetId   ***************************************************
    // *************************************************************************
    void SetId(uint32_t i) {id = i;}

    // *************************************************************************
    // ***   Public: GetId   ***************************************************
    // *************************************************************************
    uint32_t GetId() {return id;}

  private:
    static const uint8_t BORDER_W = 4u;

    // Callback function pointer
    AppTask* callback_task = nullptr;
    CallbackPtr callback_func = nullptr;
    void* callback_param = nullptr;

    // ID for caller
    uint32_t id = 0u;

    // Scale to move axis
    int32_t scale = 10u;

    // List that contains all menu elements
    VisList list;

    // Box around
    Box box;
    // Box around
    ShadowBox shadowbox;

    // String for caption
    String value_name;
    // Data windows to show value
    DataWindow value_dw;

    // Buttons to choose scale
    UiButton scale_btn[3u];
    // Scale options(value)
    const uint32_t scale_val[NumberOf(scale_btn)] = {1, 10, 100};
    // Scale options(string)
    char scale_str[NumberOf(scale_btn)][16u] = {"", "", ""};

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
    static Result ProcessEncoderCallback(ChangeValueBox* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(ChangeValueBox* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: Update function   ****************************************
    // *************************************************************************
    void UpdateObjects();

    // *************************************************************************
    // ***   Private: Power function   *****************************************
    // *************************************************************************
    int32_t Power(int32_t val, uint32_t power);
};

#endif
