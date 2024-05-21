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
// ***   ProbeScr Class   ******************************************************
// *****************************************************************************
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
    static Result ProcessButtonCallback(ProbeScr* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private: ChangeTab function   *************************************
    // *************************************************************************
    void ChangeTab(uint8_t tabn);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    ProbeScr();
};

// *****************************************************************************
// *****************************************************************************
// ***   ToolOffsetTab Class   *************************************************
// *****************************************************************************
// *****************************************************************************
class ToolOffsetTab : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static ToolOffsetTab& GetInstance();

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

    // Buttons to measure offset
    UiButton get_offset_btn;
    // Buttons to clear offset
    UiButton clear_offset_btn;
    // Buttons to measure base
    UiButton get_base_btn;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    ToolOffsetTab() {};
};

// *****************************************************************************
// *****************************************************************************
// ***   CenterFinderTab Class   ***********************************************
// *****************************************************************************
// *****************************************************************************
class CenterFinderTab : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static CenterFinderTab& GetInstance();

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
      PROBE_START,
      PROBE_X_MIN,
      PROBE_X_MAX,
      PROBE_Y_MIN,
      PROBE_Y_MAX,
      PROBE_CNT
    } probe_state_t;

    // Enum to track state
    typedef enum
    {
      PROBE_LINE_START,
      PROBE_LINE_FAST,
      PROBE_LINE_FAST_RETURN,
      PROBE_LINE_SLOW,
      PROBE_LINE_RESULT_READY, // <- this state allow to replace safe_pos with calculated center for example
      PROBE_LINE_SLOW_RETURN,
      PROBE_LINE_CNT
    } probe_line_state_t;

    // Current state
    probe_state_t state = PROBE_CNT;
    probe_line_state_t line_state = PROBE_LINE_CNT;

    // X & Y positions
    int32_t x_min_pos = 0u;
    int32_t y_min_pos = 0u;
    int32_t x_max_pos = 0u;
    int32_t y_max_pos = 0u;
    int32_t x_safe_pos = 0u;
    int32_t y_safe_pos = 0u;
    // X & Y diameters
    int32_t x_diameter = 0u;
    int32_t y_diameter = 0u;
    int32_t diameter_diviation = 0u;

//    // Current selected axis
//    GrblComm::Axis_t axis = GrblComm::AXIS_CNT;
    // ID to track send message
    uint32_t cmd_id = 0u;

    // Current GRBL state to detect changes
    GrblComm::state_t grbl_state = GrblComm::UNKNOWN;

    // Data windows to show real position
    DataWindow dw_real[GrblComm::AXIS_CNT];
    // String for caption
    String dw_real_name[NumberOf(dw_real)];

    // String for caption
    String diameter_str[3u];
    char diameter_str_buf[NumberOf(diameter_str)][64] = {0};

    // Buttons to start movement
    UiButton find_center_btn;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // *************************************************************************
    // ***   ProbeLineSequence function   **************************************
    // *************************************************************************
    Result ProbeLineSequence(probe_line_state_t& state, uint8_t axis, int32_t dir, int32_t& safe_pos, int32_t& measured_pos);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    CenterFinderTab() {};
};

#endif
