//******************************************************************************
//  @file OverrideCtrlScr.h
//  @author Nicolai Shlapunov
//
//  @details OverrideCtrlScr: User OverrideCtrlScr Class, header
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

#ifndef OverrideCtrlScr_h
#define OverrideCtrlScr_h

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
// ***   OverrideCtrlScr Class   ***********************************************
// *****************************************************************************
class OverrideCtrlScr : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static OverrideCtrlScr& GetInstance();

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

    // Data windows to show real position
    DataWindow dw_real[GrblComm::AXIS_CNT];
    // String for caption
    String dw_real_name[NumberOf(dw_real)];

    // String for caption
    String feed_name;
    // Data windows to show current value
    DataWindow feed_dw;
    // Buttons for reset feed to default
    UiButton feed_reset_btn;
    // Feed value
    int32_t feed_val = 0;

    // String for caption
    String speed_name;
    // Data windows to show current value
    DataWindow speed_dw;
    // Buttons for reset speed to default
    UiButton speed_reset_btn;
    // Feed value
    int32_t speed_val = 0;

    // Buttons for reset rapids to default
    UiButton rapids_btn[3u];

    // Buttons for control flood coolant
    UiButton flood_btn;
    // Buttons for control mist coolant
    UiButton mist_btn;

    // Soft Buttons
    UiButton left_btn;
    UiButton right_btn;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // Encoder callback entry
    InputDrv::CallbackListEntry enc_cble;
    // Button callback entry
    InputDrv::CallbackListEntry btn_cble;

    // *************************************************************************
    // ***   Private: ProcessEncoderCallback function   ************************
    // *************************************************************************
    static Result ProcessEncoderCallback(OverrideCtrlScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(OverrideCtrlScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    OverrideCtrlScr() {};
};

#endif
