//******************************************************************************
//  @file DelayControlScr.h
//  @author Nicolai Shlapunov
//
//  @details DelayControlScr: User DelayControlScr Class, header
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

#ifndef DelayControlScr_h
#define DelayControlScr_h

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
// ***   DelayControlScr Class   **********************************************
// *****************************************************************************
class DelayControlScr : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static DelayControlScr& GetInstance();

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

    // Current selected axis
    // Scale to move axis
    GrblComm::Axis_t axis = GrblComm::AXIS_CNT;
    // Scale to move axis
    int32_t scale = 10u;

    // Current GRBL state to detect changes
    GrblComm::state_t grbl_state = GrblComm::UNKNOWN;

    // String for caption
    String axis_names[GrblComm::AXIS_CNT];
    // Data windows to show DRO
    DataWindow dw[GrblComm::AXIS_CNT];
    // Data windows to show real position
    DataWindow dw_real[GrblComm::AXIS_CNT];
    // Data windows to show difference in position
    DataWindow dw_diff[GrblComm::AXIS_CNT];

    // String for caption
    String speed_name;
    // Data windows to show speed of movement
    DataWindow dw_speed;

    // Buttons to choose scale
    UiButton scale_btn[3u];

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
    static Result ProcessEncoderCallback(DelayControlScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(DelayControlScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: Update function   ****************************************
    // *************************************************************************
    void UpdateObjects();

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    DelayControlScr() {};
};

#endif
