//******************************************************************************
//  @file RotaryTableScr.h
//  @author Nicolai Shlapunov
//
//  @details RotaryTableScr: User RotaryTableScr Class, header
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

#ifndef RotaryTableScr_h
#define RotaryTableScr_h

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
// ***   RotaryTableScr Class   **********************************************
// *****************************************************************************
class RotaryTableScr : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static RotaryTableScr& GetInstance();

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

    // Focus enum
    typedef enum
    {
      FOCUS_CENTER_X_AXIS = 0u, // Should be first and start from 0
      FOCUS_CENTER_Y_AXIS = 1u, // Should be next to FOCUS_CENTER_X_AXIS
      FOCUS_RADIUS = 2u,
      FOCUS_Z_AXIS = 3u,
      FOCUS_ARC_LENGTH = 4u,
      FOCUS_CNT
    } Focus_t;

    // TODO: REMOVE!
//    Circle cir;
//    double dcx = 0;
//    double dcy = 0;
//    double dpx = 0;
//    double dpy = 0;
//    int32_t dl = 0;
    // TODO: REMOVE!

    // Current point
    double current_x = 0;
    double current_y = 0;
    // Starting point of a circle
    int32_t start_x = 0;
    int32_t start_y = 0;
    // Center coordinates
    int32_t center_x = 0;
    int32_t center_y = 0;
    // Radius
    int32_t radius = 0;
    // Radius change
    int32_t radius_change = 0;
    // Arc length from starting point
    int32_t arc_length = 0;
    // Z change
    int32_t z_val = 0;

    // Current selected object
    Focus_t focus = FOCUS_CENTER_X_AXIS;
    // Scale to move axis
    int32_t scale = 10u;

    // Current GRBL state to detect changes
    GrblComm::state_t grbl_state = GrblComm::UNKNOWN;

    // String for caption (only X and Y for center)
    String center;

    // Data windows to show real position
    DataWindow dw_real[2u];
    // String for caption (only X and Y for center)
    String dw_real_name[2u];

    // String for caption (only X and Y for center)
    String center_axis_name[2u];
    // Data windows to show DRO (only X and Y for center)
    DataWindow center_dw[2u];

    // String for caption
    String radius_name;
    // Data windows to show radius
    DataWindow radius_dw;

    // String for caption
    String z_axis_name;
    // Data windows to show Z position
    DataWindow z_axis_dw;

    // String for caption
    String arc_name;
    // Data windows to show arc length
    DataWindow arc_dw;

    // Buttons to choose scale
    UiButton scale_btn[3u];

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
    static Result ProcessEncoderCallback(RotaryTableScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(RotaryTableScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: Update function   ****************************************
    // *************************************************************************
    void UpdateObjects();

    // *************************************************************************
    // ***   Private: FindArcSecondPoint function   ****************************
    // *************************************************************************
    static void FindArcSecondPoint(double& xp, double& yp, double xc, double yc, double l, bool clockwise);

    // *****************************************************************************
    // ***   Private: FindNewPointByRadius function   ******************************
    // *****************************************************************************
    static void FindNewPointByRadius(double& xp, double& yp, double xc, double yc, double r_change);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    RotaryTableScr() {};
};

#endif
