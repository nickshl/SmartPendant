//******************************************************************************
//  @file DirectControlScr.h
//  @author Nicolai Shlapunov
//
//  @details DirectControlScr: User DirectControlScr Class, header
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

#ifndef DirectControlScr_h
#define DirectControlScr_h

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
#include "ChangeValueBox.h"

#include "Version.h"

// *****************************************************************************
// ***   Local const variables   ***********************************************
// *****************************************************************************

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************
#define BG_Z (100)

// *****************************************************************************
// ***   DirectControlScr Class   **********************************************
// *****************************************************************************
class DirectControlScr : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static DirectControlScr& GetInstance();

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
    static constexpr uint8_t BORDER_W = 4u;

    // String for version
    String version;
    // Version text with oscillator frequency
    char ver_txt[sizeof(VERSION) + 6u] = {0};
    // Value for speed
    int32_t jog_val = 0;
    // Jogging values
    int32_t axis_jog_val[GrblComm::AXIS_CNT] = {0};
    // Jogging direction
    int32_t axis_jog_dir[GrblComm::AXIS_CNT] = {0};

    // Current selected axis
    GrblComm::Axis_t axis = GrblComm::AXIS_CNT;
    // Scale to move axis
    int32_t scale = 1u;

    // String for caption
    String axis_names[GrblComm::AXIS_CNT];
    // Data windows to show DRO
    DataWindow dw[GrblComm::AXIS_CNT];
    // Buttons to set 0
    UiButton zero_btn[GrblComm::AXIS_CNT];
    // Buttons to change Radius/Diameter in Lathe Mode
    UiButton x_mode_btn;
    // String for X axis mode(Radius/Diameter)
    String x_mode_str;

    // Buttons to choose scale
    UiButton scale_btn[4u];
    // Scale options(string)
    char scale_str_metric[NumberOf(scale_btn)][12u];
    char scale_str_imperial[NumberOf(scale_btn)][12u];
    // Scale options(value)
    const uint32_t scale_val_metric[NumberOf(scale_btn)] = {1, 5, 10, 100};
    const uint32_t scale_val_imperial[NumberOf(scale_btn)] = {1, 2, 5, 50};

    // Data windows to show spindle speed
    DataWindow spindle_dw;
    // String for spindle caption
    String spindle_name;
    // Buttons to on/off spindle
    UiButton spindle_dir_btn;
    UiButton spindle_ctrl_btn;

    // Soft Buttons
    UiButton& left_btn;
    UiButton& middle_btn;
    UiButton& right_btn;

    // Object to change numerical parameters
    ChangeValueBox& change_box;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // Encoder callback entry
    InputDrv::CallbackListEntry enc_cble;

    // *************************************************************************
    // ***   Private: UnpressButtons function   ********************************
    // *************************************************************************
    void UnpressButtons();

    // *************************************************************************
    // ***   Private: ProcessEncoderCallback function   ************************
    // *************************************************************************
    static Result ProcessEncoderCallback(DirectControlScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    DirectControlScr();
};

#endif
