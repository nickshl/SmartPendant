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
#include "MsgBox.h"

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

    // *************************************************************************
    // ***   Public: EnableScreenChange function   *****************************
    // *************************************************************************
    void EnableScreenChange();

    // *************************************************************************
    // ***   Public: DisableScreenChange function   ****************************
    // *************************************************************************
    void DisableScreenChange();

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

    // Soft Buttons
    UiButton& left_btn;
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
      PROBE_DONE,
      PROBE_CNT
    } probe_state_t;

    // Enum to track state
    typedef enum
    {
      PROBE_LINE_START,
      PROBE_LINE_MOVE,
      PROBE_LINE_DIVE,
      PROBE_LINE_FAST,
      PROBE_LINE_FAST_RETURN,
      PROBE_LINE_SLOW,
      PROBE_LINE_RESULT_READY, // <- this state allow to replace safe_pos with calculated center for example
      PROBE_LINE_SLOW_RETURN,
      PROBE_LINE_ASCEND,
      PROBE_LINE_RETURN,
      PROBE_LINE_CNT
    } probe_line_state_t;
    // Current state
    probe_state_t state = PROBE_CNT;
    probe_line_state_t line_state = PROBE_LINE_CNT;

    // Enum to track iteration
    typedef enum
    {
      PROBE_ITERATION_FIRST,
      PROBE_ITERATION_MSGBOX_SHOW,
      PROBE_ITERATION_SECOND,
      PROBE_ITERATION_CNT
    } probe_iteration_t;
    // Iteration
    probe_iteration_t iteration = PROBE_ITERATION_CNT;

    // X & Y positions
    int32_t x_min_pos = 0;
    int32_t y_min_pos = 0;
    int32_t x_max_pos = 0;
    int32_t y_max_pos = 0;
    int32_t x_safe_pos = 0;
    int32_t y_safe_pos = 0;
    // X & Y distances
    int32_t x_distance = 0u;
    int32_t y_distance = 0u;
    int32_t distance_diviation = 0;
    // Copy of x_safe_pos and y_safe_pos for precise measurements
    int32_t x_first_iteration_pos = 0;
    int32_t y_first_iteration_pos = 0;

    // ID to track send message
    uint32_t cmd_id = 0u;

    // Current GRBL state to detect changes
    GrblComm::state_t grbl_state = GrblComm::UNKNOWN;

    // Data windows to show real position
    DataWindow dw_real[3u];
    // String for caption
    String dw_real_name[NumberOf(dw_real)];

    // Data windows for clearance
    DataWindow dw_clearance;
    String dw_clearance_name;
    // Data windows for distance
    DataWindow dw_distance;
    String dw_distance_name;

    // String for caption
    String data_str[3u];
    char data_str_buf[NumberOf(data_str)][64] = {0};

    // Buttons to select type of measurement
    UiButton inside_btn;
    UiButton outside_btn;

    // Button for precise measurement
    UiButton precise_btn;

    // Message box for pop up request to turn probe 180 degrees
    MsgBox msg_box;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // Encoder callback entry
    InputDrv::CallbackListEntry enc_cble;

    // *************************************************************************
    // ***   Private: ResetProbeSequence function   ****************************
    // *************************************************************************
    void ResetProbeSequence();

    // *************************************************************************
    // ***   Private: ProcessEncoderCallback function   ************************
    // *************************************************************************
    static Result ProcessEncoderCallback(CenterFinderTab* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   ProbeLineSequence function   **************************************
    // *************************************************************************
    Result ProbeLineSequence(probe_line_state_t& state, uint8_t axis, int32_t dir, int32_t& safe_pos, int32_t& measured_pos);

    // *************************************************************************
    // ***   ProbeInsideLineSequence function   ********************************
    // *************************************************************************
    Result ProbeInsideLineSequence(probe_line_state_t& state, uint8_t axis, int32_t dir, int32_t& safe_pos, int32_t& measured_pos);

    // *************************************************************************
    // ***   ProbeOutsideLineSequence function   *******************************
    // *************************************************************************
    Result ProbeOutsideLineSequence(probe_line_state_t& state, uint8_t axis, int32_t dir, int32_t len, int32_t dive, int32_t& safe_pos, int32_t& measured_pos);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    CenterFinderTab() {};
};

// *****************************************************************************
// *****************************************************************************
// ***   EdgeFinderTab Class   *************************************************
// *****************************************************************************
// *****************************************************************************
class EdgeFinderTab : public IScreen
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static EdgeFinderTab& GetInstance();

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
      PROBE_MOVE,
      PROBE_DIVE,
      PROBE_FAST,
      PROBE_FAST_RETURN,
      PROBE_SLOW,
      PROBE_RESULT_READY, // <- this state allow to replace safe_pos with calculated center for example
      PROBE_SLOW_RETURN,
      PROBE_ASCEND,
      PROBE_RETURN,
      PROBE_CNT
    } probe_state_t;
    // Current state
    probe_state_t state = PROBE_CNT;

    // Enum to track iteration
    typedef enum
    {
      PROBE_ITERATION_FIRST,
      PROBE_ITERATION_MSGBOX_SHOW,
      PROBE_ITERATION_SECOND,
      PROBE_ITERATION_CNT
    } probe_iteration_t;
    // Iteration
    probe_iteration_t iteration = PROBE_ITERATION_CNT;

    // Current probing axis
    uint8_t axis = GrblComm::AXIS_CNT;
    // Current probing direction(+1/-1)
    int8_t dir = 0;
    // Measured position
    int32_t measured_pos = 0;
    // Measured position during first iteration
    int32_t first_iteration_measured_pos = 0;
    // Safe position(from which probing started)
    int32_t safe_pos = 0;

    // ID to track send message
    uint32_t cmd_id = 0u;

    // Current GRBL state to detect changes
    GrblComm::state_t grbl_state = GrblComm::UNKNOWN;

    // Data windows to show real position
    DataWindow dw_real[3u];
    // String for caption
    String dw_real_name[NumberOf(dw_real)];

    // Data windows for clearance
    DataWindow dw_clearance;
    String dw_clearance_name;
    // Data windows for distance
    DataWindow dw_tip_diameter;
    String dw_tip_diameter_name;

    // Buttons to select type of measurement
    UiButton plus_btn;
    UiButton minus_btn;

    // Button for precise measurement
    UiButton precise_btn;

    // Message box for pop up request to turn probe 180 degrees
    MsgBox msg_box;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // GRBL Communication Interface instance
    GrblComm& grbl_comm = GrblComm::GetInstance();

    // Encoder callback entry
    InputDrv::CallbackListEntry enc_cble;

    // *************************************************************************
    // ***   Private: ResetProbeSequence function   ****************************
    // *************************************************************************
    void ResetProbeSequence();

    // *************************************************************************
    // ***   Private: ProcessEncoderCallback function   ************************
    // *************************************************************************
    static Result ProcessEncoderCallback(EdgeFinderTab* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    EdgeFinderTab() {};
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

#endif
