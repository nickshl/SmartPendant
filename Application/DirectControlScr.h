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

    // String for caption
    String version;

    // Jogging values
    int32_t jog_val[GrblComm::AXIS_CNT] = {0};

    // Current selected axis
    GrblComm::Axis_t axis = GrblComm::AXIS_CNT;
    // Scale to move axis
    int32_t scale = 1u;

    // String for caption
    String axis_names[GrblComm::AXIS_CNT];
    // Data windows to show DRO
    DataWindow dw[GrblComm::AXIS_CNT];
    // Buttons to set 0 or position
    UiButton set_btn[GrblComm::AXIS_CNT];
    // Buttons to change Radius/Diameter in Lathe Mode
    UiButton x_mode_btn;
    // String for X axis mode(Radius/Diameter)
    String x_mode_str;

    // Buttons to choose scale
    UiButton scale_btn[4u];
    // Scale options(string)
    const char scale_str_metric[NumberOf(scale_btn)][12u] = {"0.001\nmm", "0.005\nmm", "0.01\nmm", "0.1\nmm"};
    const char scale_str_imperial[NumberOf(scale_btn)][12u] = {"0.0001\ninch", "0.0002\ninch", "0.0005\ninch", "0.005\ninch"};
    // Scale options(value)
    const uint32_t scale_val_metric[NumberOf(scale_btn)] = {1, 5, 10, 100};
    const uint32_t scale_val_imperial[NumberOf(scale_btn)] = {1, 2, 5, 50};
    // Set buttons
    UiButton set_mode_btn;

    // Object to change numerical parameters
    ChangeValueBox change_box;

    // Soft Buttons
    UiButton& left_btn;
    UiButton& right_btn;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();
    // NVM instance
    NVM& nvm = NVM::GetInstance();

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
