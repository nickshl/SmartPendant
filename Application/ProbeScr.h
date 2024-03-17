//******************************************************************************
//  @file ProbeScr.h
//  @author Nicolai Shlapunov
//
//  @details ProbeScr: User ProbeScr Class, header
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

#ifndef ProbeScr_h
#define ProbeScr_h

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
// ***   ProbeScr Class   **********************************************
// *****************************************************************************
class ProbeScr : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static ProbeScr& GetInstance();

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

    // Enum to track state
    typedef enum
    {
      PROBE_BASE,
      PROBE_TOOL,
      PROBE_CNT
    } state_t;

    // Current state
    state_t state = PROBE_CNT;

    // Z position before probing
    int32_t z_position = 0u;

    // Current selected axis
    // Scale to move axis
    GrblComm::Axis_t axis = GrblComm::AXIS_CNT;
    // ID to track send message
    uint32_t cmd_id = 0u;

    // Current GRBL state to detect changes
    GrblComm::state_t grbl_state = GrblComm::UNKNOWN;

    // String for caption
    String name_tool;
    // Data windows to show tool offset
    DataWindow dw_tool;

    // String for caption
    String name_base;
    // Data windows to show tool offset
    DataWindow dw_base;

    // Data windows to show real position
    DataWindow dw_real[GrblComm::AXIS_CNT];
    // String for caption
    String dw_real_name[NumberOf(dw_real)];

    // Buttons to start movement
    UiButton get_offset_btn;
    // Buttons to reset changed values to current
    UiButton get_base_btn;

    // Soft Button
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
    static Result ProcessEncoderCallback(ProbeScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(ProbeScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: Update function   ****************************************
    // *************************************************************************
    void UpdateObjects();

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    ProbeScr() {};
};

#endif
