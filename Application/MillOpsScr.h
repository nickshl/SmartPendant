//******************************************************************************
//  @file MillOpsScr.h
//  @author Nicolai Shlapunov
//
//  @details MillOpsScr: User MillOpsScr Class, header
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

#ifndef MillOpsScr_h
#define MillOpsScr_h

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
#include "Tabs.h"

// *****************************************************************************
// ***   Local const variables   ***********************************************
// *****************************************************************************

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************
#define BG_Z (100)

// *****************************************************************************
// *****************************************************************************
// ***   MillOpsScr Class   ****************************************************
// *****************************************************************************
// *****************************************************************************
class MillOpsScr : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static MillOpsScr& GetInstance();

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

    // Pages
    Tabs tabs;

    // Pointers to screens
    IScreen* tab[16u] = {0};
    // Screen counter
    uint32_t tabs_cnt = 0u;
    // Current screen index
    uint32_t tab_idx = 0u;

    // Soft Button
    UiButton& right_btn;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // Button callback entry
    InputDrv::CallbackListEntry btn_cble;

    // *************************************************************************
    // ***   Private: ProcessButtonCallback function   *************************
    // *************************************************************************
    static Result ProcessButtonCallback(MillOpsScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ChangeTab function   *************************************
    // *************************************************************************
    void ChangeTab(uint8_t tabn);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    MillOpsScr();
};

// *****************************************************************************
// *****************************************************************************
// ***   DrillGeneratorTab Class   *********************************************
// *****************************************************************************
// *****************************************************************************
class DrillGeneratorTab : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static DrillGeneratorTab& GetInstance();

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

    // Scale to move axis
    int32_t scale = 10u;

    // String for caption
    String dw_drill_distance_name;
    String dw_drill_stepover_name;
    String dw_drill_clearance_name;
    String dw_drill_feed_name;
    // Data windows
    DataWindow dw_drill_distance;
    DataWindow dw_drill_stepover;
    DataWindow dw_drill_clearance;
    DataWindow dw_drill_feed;

    // Buttons to choose scale
    UiButton scale_btn[3u];
    // Scale options(string)
    const char scale_str_metric[NumberOf(scale_btn)][9u] = {"0.001\nmm", "0.01\nmm", "0.1\nmm"};
    const char scale_str_imperial[NumberOf(scale_btn)][12u] = {"0.0001\ninch", "0.001\ninch", "0.01\ninch"};

    // Soft Buttons
    UiButton& left_btn;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // Encoder callback entry
    InputDrv::CallbackListEntry enc_cble;

    // *************************************************************************
    // ***   Private: ProcessEncoderCallback function   ************************
    // *************************************************************************
    static Result ProcessEncoderCallback(DrillGeneratorTab* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: GenerateGcode function   *********************************
    // *************************************************************************
    Result GenerateGcode();

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    DrillGeneratorTab();
};

// *****************************************************************************
// *****************************************************************************
// ***   EnlargeGeneratorTab Class   *******************************************
// *****************************************************************************
// *****************************************************************************
class EnlargeGeneratorTab : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static EnlargeGeneratorTab& GetInstance();

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

    // Scale to move axis
    int32_t scale = 10u;

    // String for caption
    String dw_hole_diameter_name;
    String dw_stepover_name;
    String dw_endmill_diameter_name;
    String dw_feed_name;
    // Data windows
    DataWindow dw_hole_diameter;
    DataWindow dw_stepover;
    DataWindow dw_endmill_diameter;
    DataWindow dw_feed;
    // Buttons to choose direction
    UiButton direction_btn;
    const char direction_str[2u][9u] = {"CW", "CCW"};
    bool direction = false;

    // Buttons to choose scale
    UiButton scale_btn[3u];
    // Scale options(string)
    const char scale_str_metric[NumberOf(scale_btn)][9u] = {"0.001\nmm", "0.01\nmm", "0.1\nmm"};
    const char scale_str_imperial[NumberOf(scale_btn)][12u] = {"0.0001\ninch", "0.001\ninch", "0.01\ninch"};

    // Soft Buttons
    UiButton& left_btn;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // Encoder callback entry
    InputDrv::CallbackListEntry enc_cble;

    // *************************************************************************
    // ***   Private: ProcessEncoderCallback function   ************************
    // *************************************************************************
    static Result ProcessEncoderCallback(EnlargeGeneratorTab* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: GenerateGcode function   *********************************
    // *************************************************************************
    Result GenerateGcode();

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    EnlargeGeneratorTab();
};

#endif
