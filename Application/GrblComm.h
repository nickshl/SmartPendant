//******************************************************************************
//  @file GrblComm.h
//  @author Nicolai Shlapunov
//
//  @details GrblComm: Grbl Communication Class, header
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

#ifndef GrblComm_h
#define GrblComm_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "AppTask.h"
#include "StHalUart.h"

#include "NVM.h"

// *****************************************************************************
// ***   Debug defines   *******************************************************
// *****************************************************************************
//#define SEND_DATA_TO_USB

// *****************************************************************************
// ***   GrblComm Class   ******************************************************
// *****************************************************************************
class GrblComm : public AppTask
{
  public:
    // *************************************************************************
    // ***   Tx Control Enum   *************************************************
    // *************************************************************************
    typedef enum : uint8_t
    {
      CTRL_GPIO_PIN = 0u,  // Dedicated pin(open drain) is used for MPG control toggle
      CTRL_SW_COMMAND,     // CMD_MPG_MODE_TOGGLE(0x8B) is used for MPG control toggle
      CTRL_PIN_AND_SW_CMD, // Both dedicated pin and CMD_MPG_MODE_TOGGLE(0x8B) is used for MPG control toggle(for LED indication)
      CTRL_FULL,           // Full control if MPG connected to primary UART
      CTRL_TX_CNT
    } ctrl_tx_t;

    // *************************************************************************
    // ***   State Enum   ******************************************************
    // *************************************************************************
    typedef enum : uint8_t
    {
      UNKNOWN = 0,
      IDLE,
      RUN,
      JOG,
      HOLD,
      ALARM,
      CHECK,
      DOOR,
      TOOL,
      HOME,
      SLEEP,
      STATE_CNT
    } state_t;

    // *************************************************************************
    // ***   Axis Index Enum   *************************************************
    // *************************************************************************
    typedef enum
    {
       AXIS_X = 0,
       AXIS_Y = 1,
       AXIS_Z = 2,
       AXIS_A = 3,
       AXIS_B = 4,
       AXIS_C = 5,
       AXIS_CNT
    } Axis_t;

    // *************************************************************************
    // ***   Real Time Commands Enum   *****************************************
    // *************************************************************************
    enum : uint8_t
    {
      // Define realtime command special characters. These characters are 'picked-off' directly from the
      // serial read data stream and are not passed to the grbl line execution parser. Select characters
      // that do not and must not exist in the streamed g-code program. ASCII control characters may be
      // used, if they are available per user setup. Also, extended ASCII codes (>127), which are never in
      // g-code programs, maybe selected for interface programs.
      // NOTE: If changed, manually update help message in report.c.
      CMD_EXIT = 0x03,   // ctrl-C (ETX)
      CMD_REBOOT = 0x14, // ctrl-T (DC4) - only acted upon if preceded by 0x1B (ESC)
      CMD_RESET = 0x18,  // ctrl-X (CAN)
      CMD_STOP = 0x19,   // ctrl-Y (EM)
      CMD_STATUS_REPORT_LEGACY = '?',
      CMD_CYCLE_START_LEGACY = '~',
      CMD_FEED_HOLD_LEGACY = '!',
      CMD_PROGRAM_DEMARCATION = '%',

      // NOTE: All override realtime commands must be in the extended ASCII character set, starting
      // at character value 128 (0x80) and up to 255 (0xFF). If the normal set of realtime commands,
      // such as status reports, feed hold, reset, and cycle start, are moved to the extended set
      // space, protocol.c's protocol_process_realtime() will need to be modified to accommodate the change.
      CMD_STATUS_REPORT = 0x80, // TODO: use 0x05 ctrl-E ENQ instead?
      CMD_CYCLE_START = 0x81,   // TODO: use 0x06 ctrl-F ACK instead? or SYN/DC2/DC3?
      CMD_FEED_HOLD = 0x82,     // TODO: use 0x15 ctrl-U NAK instead?
      CMD_GCODE_REPORT = 0x83,
      CMD_SAFETY_DOOR = 0x84,
      CMD_JOG_CANCEL  = 0x85,
//      CMD_DEBUG_REPORT = 0x86, // Only when DEBUG enabled, sends debug report in '{}' braces.
      CMD_STATUS_REPORT_ALL = 0x87,
      CMD_OPTIONAL_STOP_TOGGLE = 0x88,
      CMD_SINGLE_BLOCK_TOGGLE = 0x89,
      CMD_OVERRIDE_FAN0_TOGGLE = 0x8A,       // Toggle Fan 0 on/off, not implemented by the core.
      CMD_MPG_MODE_TOGGLE = 0x8B,            // Toggle MPG mode on/off
      CMD_AUTO_REPORTING_TOGGLE = 0x8C,      // Toggle auto real time reporting if configured.
      CMD_OVERRIDE_FEED_RESET = 0x90,        // Restores feed override value to 100%.
      CMD_OVERRIDE_FEED_COARSE_PLUS = 0x91,
      CMD_OVERRIDE_FEED_COARSE_MINUS = 0x92,
      CMD_OVERRIDE_FEED_FINE_PLUS = 0x93,
      CMD_OVERRIDE_FEED_FINE_MINUS = 0x94,
      CMD_OVERRIDE_RAPID_RESET = 0x95,       // Restores rapid override value to 100%.
      CMD_OVERRIDE_RAPID_MEDIUM = 0x96,
      CMD_OVERRIDE_RAPID_LOW = 0x97,
//      CMD_OVERRIDE_RAPID_EXTRA_LOW = 0x98, // *NOT SUPPORTED*
      CMD_OVERRIDE_SPINDLE_RESET = 0x99,     // Restores spindle override value to 100%.
      CMD_OVERRIDE_SPINDLE_COARSE_PLUS = 0x9A,
      CMD_OVERRIDE_SPINDLE_COARSE_MINUS = 0x9B,
      CMD_OVERRIDE_SPINDLE_FINE_PLUS = 0x9C,
      CMD_OVERRIDE_SPINDLE_FINE_MINUS = 0x9D,
      CMD_OVERRIDE_SPINDLE_STOP = 0x9E,
      CMD_OVERRIDE_COOLANT_FLOOD_TOGGLE = 0xA0,
      CMD_OVERRIDE_COOLANT_MIST_TOGGLE = 0xA1,
      CMD_PID_REPORT = 0xA2,
      CMD_TOOL_ACK = 0xA3,
      CMD_PROBE_CONNECTED_TOGGLE = 0xA4,
    };

    // *************************************************************************
    // ***   Alarm executor codes. Zero is reserved.   *************************
    // *************************************************************************
    typedef enum : uint8_t
    {
      Alarm_None = 0,                             //!< 0
      Alarm_HardLimit = 1,                        //!< 1
      Alarm_SoftLimit = 2,                        //!< 2
      Alarm_AbortCycle = 3,                       //!< 3
      Alarm_ProbeFailInitial = 4,                 //!< 4
      Alarm_ProbeFailContact = 5,                 //!< 5
      Alarm_HomingFailReset = 6,                  //!< 6
      Alarm_HomingFailDoor = 7,                   //!< 7
      Alarm_FailPulloff = 8,                      //!< 8
      Alarm_HomingFailApproach = 9,               //!< 9
      Alarm_EStop = 10,                           //!< 10
      Alarm_HomingRequired = 11,                  //!< 11
      Alarm_LimitsEngaged = 12,                   //!< 12
      Alarm_ProbeProtect = 13,                    //!< 13
      Alarm_Spindle = 14,                         //!< 14
      Alarm_HomingFailAutoSquaringApproach = 15,  //!< 15
      Alarm_SelftestFailed = 16,                  //!< 16
      Alarm_MotorFault = 17,                      //!< 17
      Alarm_HomingFail = 18,                      //!< 18
      Alarm_Code_Cnt
    } alarm_code_t;

    // *************************************************************************
    // ***   Error Codes Enum   ************************************************
    // *************************************************************************
    typedef enum : uint8_t
    {
      Status_OK = 0,
      Status_ExpectedCommandLetter = 1,
      Status_BadNumberFormat = 2,
      Status_InvalidStatement = 3,
      Status_NegativeValue = 4,
      Status_HomingDisabled = 5,
      Status_SettingStepPulseMin = 6,
      Status_SettingReadFail = 7,
      Status_IdleError = 8,
      Status_SystemGClock = 9,
      Status_SoftLimitError = 10,
      Status_Overflow = 11,
      Status_MaxStepRateExceeded = 12,
      Status_CheckDoor = 13,
      Status_LineLengthExceeded = 14,
      Status_TravelExceeded = 15,
      Status_InvalidJogCommand = 16,
      Status_SettingDisabledLaser = 17,
      Status_Reset = 18,
      Status_NonPositiveValue = 19,

      Status_GcodeUnsupportedCommand = 20,
      Status_GcodeModalGroupViolation = 21,
      Status_GcodeUndefinedFeedRate = 22,
      Status_GcodeCommandValueNotInteger = 23,
      Status_GcodeAxisCommandConflict = 24,
      Status_GcodeWordRepeated = 25,
      Status_GcodeNoAxisWords = 26,
      Status_GcodeInvalidLineNumber = 27,
      Status_GcodeValueWordMissing = 28,
      Status_GcodeUnsupportedCoordSys = 29,
      Status_GcodeG53InvalidMotionMode = 30,
      Status_GcodeAxisWordsExist = 31,
      Status_GcodeNoAxisWordsInPlane = 32,
      Status_GcodeInvalidTarget = 33,
      Status_GcodeArcRadiusError = 34,
      Status_GcodeNoOffsetsInPlane = 35,
      Status_GcodeUnusedWords = 36,
      Status_GcodeG43DynamicAxisError = 37,
      Status_GcodeIllegalToolTableEntry = 38,
      Status_GcodeValueOutOfRange = 39,
      Status_GcodeToolChangePending = 40,
      Status_GcodeSpindleNotRunning = 41,
      Status_GcodeIllegalPlane = 42,
      Status_GcodeMaxFeedRateExceeded = 43,
      Status_GcodeRPMOutOfRange = 44,
      Status_LimitsEngaged = 45,
      Status_HomingRequired = 46,
      Status_GCodeToolError = 47,
      Status_ValueWordConflict = 48,
      Status_SelfTestFailed = 49,
      Status_EStop = 50,
      Status_MotorFault = 51,
      Status_SettingValueOutOfRange = 52,
      Status_SettingDisabled = 53,
      Status_GcodeInvalidRetractPosition = 54,
      Status_IllegalHomingConfiguration = 55,
      Status_GCodeCoordSystemLocked = 56,

      // Some error codes as defined in bdring's ESP32 port
      Status_SDMountError = 60,
      Status_SDReadError = 61,
      Status_SDFailedOpenDir = 62,
      Status_SDDirNotFound = 63,
      Status_SDFileEmpty = 64,

      Status_BTInitError = 70,

      //
      Status_ExpressionUknownOp = 71,
      Status_ExpressionDivideByZero = 72,
      Status_ExpressionArgumentOutOfRange = 73,
      Status_ExpressionInvalidArgument = 74,
      Status_ExpressionSyntaxError = 75,
      Status_ExpressionInvalidResult = 76,

      Status_AuthenticationRequired = 77,
      Status_AccessDenied = 78,
      Status_NotAllowedCriticalEvent = 79,

      Status_FlowControlNotExecutingMacro = 80,
      Status_FlowControlSyntaxError = 81,
      Status_FlowControlStackOverflow = 82,
      Status_FlowControlOutOfMemory = 83,

      // ***   For internal use only   *****************************************
      Status_Next_Cmd_Executed,    // This status show that next command send and status for requested command is lost
      Status_Cmd_Not_Executed_Yet, // Requested command isn't send to controller yet
      Status_Comm_Error,           // We lost control or controller isn't responded in time to status request
      Status_Unhandled,
      Status_Status_Cnt
    } status_t;

    // *************************************************************************
    // ***   Mode of Operation Enum   ******************************************
    // *************************************************************************
    typedef enum : uint8_t
    {
      MODE_OF_OPERATION_MILL = 0u,
      MODE_OF_OPERATION_LASER,
      MODE_OF_OPERATION_LATHE,
      MODE_OF_OPERATION_CNT
    } mode_of_operation_t;

    // *************************************************************************
    // ***   Measurement system   **********************************************
    // *************************************************************************
    typedef enum : uint8_t
    {
      MEASUREMENT_SYSTEM_METRIC = 0u,
      MEASUREMENT_SYSTEM_IMPERIAL,
      MEASUREMENT_SYSTEM_CNT
    } measurement_system_t;

    // *************************************************************************
    // ***   Public: Get Instance   ********************************************
    // *************************************************************************
    static GrblComm& GetInstance(void);

    // *************************************************************************
    // ***   Public: Init GrblComm Task   **************************************
    // *************************************************************************
    Result InitTask(StHalUart& uart_in);

    // *************************************************************************
    // ***   Public: Setup function   ******************************************
    // *************************************************************************
    virtual Result Setup();

    // *************************************************************************
    // ***   Public: TimerExpired function   ***********************************
    // *************************************************************************
    virtual Result TimerExpired(uint32_t missed_cnt);

    // *************************************************************************
    // ***   Public: ProcessMessage function   *********************************
    // *************************************************************************
    virtual Result ProcessMessage();

    // *************************************************************************
    // ***   Public: GainControl function   ************************************
    // *************************************************************************
    void GainControl();

    // *************************************************************************
    // ***   Public: ReleaseControl function   *********************************
    // *************************************************************************
    void ReleaseControl();

    // *************************************************************************
    // ***   Public: GetFullControlMode function   *****************************
    // *************************************************************************
    bool GetFullControlMode() {return(NVM::GetInstance().GetCtrlTx() == CTRL_FULL);}

    // *************************************************************************
    // ***   Public: IsInControl function   ************************************
    // *************************************************************************
    bool IsInControl() {return (grbl_mpgMode || GetFullControlMode());}

    // *************************************************************************
    // ***   Public: GetMpgMode function   *************************************
    // *************************************************************************
    bool GetMpgMode() {return grbl_mpgMode;}

    // *************************************************************************
    // ***   Public: GetMpgModeRequest function   ******************************
    // *************************************************************************
    bool GetMpgModeRequest() {return mpg_mode_request;}

    // *************************************************************************
    // ***   Public: GetNumberOfAxis function   ********************************
    // *************************************************************************
    uint32_t GetNumberOfAxis() {return number_of_axis;}

    // *************************************************************************
    // ***   Public: GetLimitedNumberOfAxis function   *************************
    // *************************************************************************
    uint32_t GetLimitedNumberOfAxis(uint32_t n) {return (((uint32_t)number_of_axis < n) ? number_of_axis : n);}

    // *************************************************************************
    // ***   Public: GetAxisName function   ************************************
    // *************************************************************************
    const char* const GetAxisName(uint8_t axis);

    // *************************************************************************
    // ***   Public: GetState function   ***************************************
    // *************************************************************************
    state_t GetState() {return grbl_state;}

    // *************************************************************************
    // ***   Public: GetPinsStr function   *************************************
    // *************************************************************************
    const char* const GetPinsStr() {return grbl_pins;}

    // *************************************************************************
    // ***   Public: IsPinsStrChanged function   *******************************
    // *************************************************************************
    bool IsPinsStrChanged() {bool ret = grbl_changed.pins; grbl_changed.pins = false; return ret;}

    // *************************************************************************
    // ***   Public: GetStateName function   ***********************************
    // *************************************************************************
    const char* const GetStateName(state_t state);

    // *************************************************************************
    // ***   Public: GetCurrentStateName function   ****************************
    // *************************************************************************
    inline const char* const GetCurrentStateName() {return GetStateName(grbl_state);}

    // *************************************************************************
    // ***   Public: GetStatusCode function   **********************************
    // *************************************************************************
    status_t GetStatusCode() {return grbl_status;}

    // *************************************************************************
    // ***   Public: GetStatusName function   **********************************
    // *************************************************************************
    const char* const GetStatusName(status_t status);

    // *************************************************************************
    // ***   Public: GetCurrentStatusName function   ***************************
    // *************************************************************************
    const char* const GetCurrentStatusName() {return GetStatusName(grbl_status);}

    // *************************************************************************
    // ***   Public: IsSettingsChanged function   ******************************
    // *************************************************************************
    bool IsSettingsChanged() {bool result = settings_changed; settings_changed = false; return result;}

    // *************************************************************************
    // ***   Public: IsMetric function   ***************************************
    // *************************************************************************
    inline bool IsMetric() {return (measurement_system == MEASUREMENT_SYSTEM_METRIC);}

    // *************************************************************************
    // ***   Public: GetUnitsScaler function   *********************************
    // *************************************************************************
    inline int32_t GetUnitsScaler() {return (IsMetric() ? 1000 : 10000);} // 1 um for metric(base unit mm) / 1 tenths for imperial(base unit inch)

    // *************************************************************************
    // ***   Public: GetSpeedScaler function   *********************************
    // *************************************************************************
    inline int32_t GetSpeedScaler() {return (IsMetric() ? 1 : 1);} // 1 mm/min or 1 inch/min

    // *************************************************************************
    // ***   Public: GetUnitsPrecision function   *********************************
    // *************************************************************************
    inline uint32_t GetUnitsPrecision() {return (IsMetric() ? 3u : 4u);} // 0.000 for metric / 0.0000 for imperial

    // *************************************************************************
    // ***   Public: ConvertMetricToUnits function   ***************************
    // *************************************************************************
    inline int32_t ConvertMetricToUnits(int32_t metric) {return (IsMetric() ? metric : metric * 100 / 254);}

    // *************************************************************************
    // ***   Public: ConvertUnitsToMetric function   ***************************
    // *************************************************************************
    inline int32_t ConvertUnitsToMetric(int32_t units) {return (IsMetric() ? units : units * 254 / 100);}

    // *************************************************************************
    // ***   Public: GetMeasurementSystem function   ***************************
    // *************************************************************************
    uint8_t GetMeasurementSystem() {return measurement_system;}

    // *************************************************************************
    // ***   Public: GetReportUnits function   *********************************
    // *************************************************************************
    inline const char* GetReportUnits() {return ((measurement_system == MEASUREMENT_SYSTEM_METRIC) ? "mm" : "inch");}

    // *************************************************************************
    // ***   Public: GetReportSpeedUnits function   ****************************
    // *************************************************************************
    inline const char* GetReportSpeedUnits() {return ((measurement_system == MEASUREMENT_SYSTEM_METRIC) ? "mm/min" : "inches/min");}

    // *************************************************************************
    // ***   Public: ValueToStringWithScaler function   ************************
    // *************************************************************************
    inline char* ValueToStringWithScaler(char* buf, uint32_t buf_size, int32_t val, int32_t scaler) {return ValueToString(buf, buf_size, val, scaler);}

    // *************************************************************************
    // ***   Public: ValueToStringWithScalerAndUnits function   ****************
    // *************************************************************************
    inline char* ValueToStringWithScalerAndUnits(char* buf, uint32_t buf_size, int32_t val, int32_t scaler, const char* units) {return ValueToStringWithUnits(buf, buf_size, val, scaler, units);}

    // *************************************************************************
    // ***   Public: ValueToStringInCurrentUnits function   ********************
    // *************************************************************************
    inline char* ValueToStringInCurrentUnits(char* buf, uint32_t buf_size, int32_t val) {return ValueToString(buf, buf_size, val, GetUnitsScaler());}

    // *************************************************************************
    // ***   Public: GetModeOfOperation function   *****************************
    // *************************************************************************
    uint8_t GetModeOfOperation() {return mode_of_operation;}

    // *************************************************************************
    // ***   Public: IsHomingEnabled function   ********************************
    // *************************************************************************
    bool IsHomingEnabled() {return(homing & 0x0001u);}

    // *************************************************************************
    // ***   Public: GetAxisMachinePosition function   *************************
    // *************************************************************************
    int32_t GetAxisMachinePosition(uint8_t axis);

    // *************************************************************************
    // ***   Public: GetAxisPosition function   ********************************
    // *************************************************************************
    int32_t GetAxisPosition(uint8_t axis);

    // *************************************************************************
    // ***   Public: GetProbeMachinePosition function   ************************
    // *************************************************************************
    int32_t GetProbeMachinePosition(uint8_t axis);

    // *************************************************************************
    // ***   Public: IsProbeTriggered function   *******************************
    // *************************************************************************
    bool IsProbeTriggered() {return grbl_probe_triggered;}

    // *************************************************************************
    // ***   Public: GetProbePosition function   *******************************
    // *************************************************************************
    int32_t GetProbePosition(uint8_t axis);

    // *************************************************************************
    // ***   Public: GetToolLengthOffset function   ****************************
    // *************************************************************************
    int32_t GetToolLengthOffset() {return (int32_t)(grbl_tool_length_offset[AXIS_Z] * GetUnitsScaler());}

    // *************************************************************************
    // ***   Public: GetFeedOverride function   ********************************
    // *************************************************************************
    int32_t GetFeedOverride() {return grbl_feed_override;}

    // *************************************************************************
    // ***   Public: GetSpeedOverride function   *******************************
    // *************************************************************************
    int32_t GetSpeedOverride() {return spindle_rpm_override;}

    // *************************************************************************
    // ***   Public: GetCoolantFlood function   ********************************
    // *************************************************************************
    bool GetCoolantFlood() {return coolant_flood;}

    // *************************************************************************
    // ***   Public: GetCoolantMist function   *********************************
    // *************************************************************************
    bool GetCoolantMist() {return coolant_mist;}

    // *************************************************************************
    // ***   Public: GetSpindleMaxSpeed function   *****************************
    // *************************************************************************
    uint16_t GetSpindleMaxSpeed() {return spindle_speed_max;}

    // *************************************************************************
    // ***   Public: GetSpindleMaxSpeed function   *****************************
    // *************************************************************************
    uint16_t GetSpindleMinSpeed() {return spindle_speed_min;}

    // *************************************************************************
    // ***   Public: IsSpindleRunning function   *******************************
    // *************************************************************************
    bool IsSpindleRunning() {return spindle_on;}

    // *************************************************************************
    // ***   Public: IsSpindleCCW function   ***********************************
    // *************************************************************************
    bool IsSpindleCCW() {return spindle_ccw;}

    // *************************************************************************
    // ***   Public: GetSpindleSpeed function   ********************************
    // *************************************************************************
    uint16_t GetSpindleSpeed() {return spindle_rpm_programmed;}

    // *************************************************************************
    // ***   Public: GetSpindleActualSpeed function   **************************
    // *************************************************************************
    uint16_t GetSpindleActualSpeed() {return spindle_rpm_actual;}

    // *************************************************************************
    // ***   Public: IsLatheDiameterMode function   ****************************
    // *************************************************************************
    bool IsLatheDiameterMode() {return grbl_xModeDiameter;}

    // *************************************************************************
    // ***   Public: IsRespondPending function   *******************************
    // *************************************************************************
    bool IsRespondPending() {return respond_pending;}

    // *************************************************************************
    // ***   Public: GetCmdResult   ********************************************
    // *************************************************************************
    status_t GetCmdResult(uint32_t id);

    // *************************************************************************
    // ***   Public: IsStatusReceivedAfterCmd   ********************************
    // *************************************************************************
    bool IsStatusReceivedAfterCmd(uint32_t id);

    // *************************************************************************
    // ***   Public: SendCmd   *************************************************
    // *************************************************************************
    Result SendCmd(const char* cmd, uint32_t &id);

    // *************************************************************************
    // ***   Public: SendRealTimeCmd   *****************************************
    // *************************************************************************
    Result SendRealTimeCmd(uint8_t cmd);

    // *************************************************************************
    // ***   Public: SendCmd   *************************************************
    // *************************************************************************
    inline Result SendCmd(const char* cmd) {uint32_t id = 0u; return SendCmd(cmd, id);} // TODO: remove! All callers have to receive ID... or may be not

    // *************************************************************************
    // ***   Public: UpdateStatus function   ***********************************
    // *************************************************************************
    void UpdateStatus();

    // *************************************************************************
    // ***   Public: Run   *****************************************************
    // *************************************************************************
    inline Result Run() {return SendRealTimeCmd(CMD_CYCLE_START);}

    // *************************************************************************
    // ***   Public: Hold   ****************************************************
    // *************************************************************************
    inline Result Hold() {return SendRealTimeCmd(CMD_FEED_HOLD);}

    // *************************************************************************
    // ***   Public: Stop   ****************************************************
    // *************************************************************************
    inline Result Stop() {respond_pending = false; send_id = next_id; return SendRealTimeCmd(CMD_STOP);}

    // *************************************************************************
    // ***   Public: Reset   ***************************************************
    // *************************************************************************
    inline Result Reset() {grbl_status = Status_OK; respond_pending = false; send_id = next_id; return SendRealTimeCmd(CMD_RESET);}

    // *************************************************************************
    // ***   Public: FeedReset   ***********************************************
    // *************************************************************************
    inline Result FeedReset() {return SendRealTimeCmd(CMD_OVERRIDE_FEED_RESET);}

    // *************************************************************************
    // ***   Public: FeedCoarsePlus   ******************************************
    // *************************************************************************
    inline Result FeedCoarsePlus() {return SendRealTimeCmd(CMD_OVERRIDE_FEED_COARSE_PLUS);}

    // *************************************************************************
    // ***   Public: FeedCoarseMinus   *****************************************
    // *************************************************************************
    inline Result FeedCoarseMinus() {return SendRealTimeCmd(CMD_OVERRIDE_FEED_COARSE_MINUS);}

    // *************************************************************************
    // ***   Public: FeedFinePlus   ********************************************
    // *************************************************************************
    inline Result FeedFinePlus() {return SendRealTimeCmd(CMD_OVERRIDE_FEED_FINE_PLUS);}

    // *************************************************************************
    // ***   Public: FeedFineMinus   *******************************************
    // *************************************************************************
    inline Result FeedFineMinus() {return SendRealTimeCmd(CMD_OVERRIDE_FEED_FINE_MINUS);}

    // *************************************************************************
    // ***   Public: SpeedReset   **********************************************
    // *************************************************************************
    inline Result SpeedReset() {return SendRealTimeCmd(CMD_OVERRIDE_SPINDLE_RESET);}

    // *************************************************************************
    // ***   Public: SpeedCoarsePlus   *****************************************
    // *************************************************************************
    inline Result SpeedCoarsePlus() {return SendRealTimeCmd(CMD_OVERRIDE_SPINDLE_COARSE_PLUS);}

    // *************************************************************************
    // ***   Public: SpeedCoarseMinus   ****************************************
    // *************************************************************************
    inline Result SpeedCoarseMinus() {return SendRealTimeCmd(CMD_OVERRIDE_SPINDLE_COARSE_MINUS);}

    // *************************************************************************
    // ***   Public: SpeedFinePlus   *******************************************
    // *************************************************************************
    inline Result SpeedFinePlus() {return SendRealTimeCmd(CMD_OVERRIDE_SPINDLE_FINE_PLUS);}

    // *************************************************************************
    // ***   Public: SpeedFineMinus   ******************************************
    // *************************************************************************
    inline Result SpeedFineMinus() {return SendRealTimeCmd(CMD_OVERRIDE_SPINDLE_FINE_MINUS);}

    // *************************************************************************
    // ***   Public: CoolantFloodToggle   **************************************
    // *************************************************************************
    inline Result CoolantFloodToggle() {return SendRealTimeCmd(CMD_OVERRIDE_COOLANT_FLOOD_TOGGLE);}

    // *************************************************************************
    // ***   Public: CoolantMistToggle   ***************************************
    // *************************************************************************
    inline Result CoolantMistToggle() {return SendRealTimeCmd(CMD_OVERRIDE_COOLANT_MIST_TOGGLE);}

    // *************************************************************************
    // ***   Public: Unlock   **************************************************
    // *************************************************************************
    inline Result Unlock() {grbl_status = Status_OK; respond_pending = false; send_id = next_id; return SendCmd("$X\r");}

    // *************************************************************************
    // ***   Public: RequestControllerParameters   *****************************
    // *************************************************************************
    inline Result RequestControllerParameters() {return SendCmd("$I\r");}

    // *************************************************************************
    // ***   Public: RequestControllerSettings   *******************************
    // *************************************************************************
    inline Result RequestControllerSettings() {return SendCmd("$$\r");}

    // *************************************************************************
    // ***   Public: RequestOffsets   ******************************************
    // *************************************************************************
    inline Result RequestOffsets()  {return SendCmd("$#\r");}

    // *************************************************************************
    // ***   Public: Homing   **************************************************
    // *************************************************************************
    inline Result Homing() {return SendCmd("$H\r");}

    // *************************************************************************
    // ***   Public: SetAbsoluteMode   *****************************************
    // *************************************************************************
    inline Result SetAbsoluteMode() {return SendCmd("G90\r");}

    // *************************************************************************
    // ***   Public: SetIncrementalMode   **************************************
    // *************************************************************************
    inline Result SetIncrementalMode() {return SendCmd("G91\r");}

    // *************************************************************************
    // ***   Public: SetLatheRadiusMode   **************************************
    // *************************************************************************
    inline Result SetLatheRadiusMode() {return SendCmd("G8\r");}

    // *************************************************************************
    // ***   Public: SetLatheDiameterMode   ************************************
    // *************************************************************************
    inline Result SetLatheDiameterMode() {return SendCmd("G7\r");}

    // *************************************************************************
    // ***   Public: Jog   *****************************************************
    // *************************************************************************
    Result Jog(uint8_t axis, int32_t distance, uint32_t feed_x100, bool is_absolute);

    // *************************************************************************
    // ***   Public: JogInMachineCoodinates   **********************************
    // *************************************************************************
    Result JogInMachineCoodinates(uint8_t axis, int32_t distance, uint32_t feed_x100);

    // *************************************************************************
    // ***   Public: JogMultiple   *********************************************
    // *************************************************************************
    Result JogMultiple(int32_t distance_x, int32_t distance_y, int32_t distance_z, uint32_t feed_x100, bool is_absolute);

    // *************************************************************************
    // ***   Public: JogArcXY   ************************************************
    // *************************************************************************
    Result JogArcXYR(int32_t x, int32_t y, uint32_t r, uint32_t feed_x100, bool direction, bool is_absolute);

    // *************************************************************************
    // ***   Public: ZeroAxis   ************************************************
    // *************************************************************************
    Result ZeroAxis(uint8_t axis);

    // *************************************************************************
    // ***   Public: SetAxisPosition   *****************************************
    // *************************************************************************
    Result SetAxisPosition(uint8_t axis, int32_t position);

    // *************************************************************************
    // ***   Public: MoveAxis   ************************************************
    // *************************************************************************
    Result MoveAxis(uint8_t axis, int32_t distance, uint32_t feed_x100, uint32_t &id);

    // *************************************************************************
    // ***   Public: ProbeAxisTowardWorkpiece   ********************************
    // *************************************************************************
    Result ProbeAxisTowardWorkpiece(uint8_t axis, int32_t position, uint32_t feed, uint32_t &id, bool strict = true);

    // *************************************************************************
    // ***   Public: ProbeAxisAwayFromWorkpiece   ******************************
    // *************************************************************************
    Result ProbeAxisAwayFromWorkpiece(uint8_t axis, int32_t position, uint32_t feed, uint32_t &id);

    // *************************************************************************
    // ***   Public: SetToolLengthOffset   *************************************
    // *************************************************************************
    Result SetToolLengthOffset(int32_t offset);

    // *************************************************************************
    // ***   Public: ClearToolLengthOffset   ***********************************
    // *************************************************************************
    Result ClearToolLengthOffset();

    // *************************************************************************
    // ***   Public: SetSpindleSpeed   *****************************************
    // *************************************************************************
    Result SetSpindleSpeed(int32_t speed, bool dir_ccw);

    // *************************************************************************
    // ***   Public: SetSpindleDirection   *************************************
    // *************************************************************************
    Result SetSpindleDirection(bool dir_ccw);

    // *************************************************************************
    // ***   Public: StopSpindle   *********************************************
    // *************************************************************************
    Result StopSpindle();

    // *************************************************************************
    // ***   Public: ValueToString function   **********************************
    // *************************************************************************
    char* ValueToString(char* buf, uint32_t buf_size, int32_t val, int32_t scaler);

    // *****************************************************************************
    // ***   Public: ValueToStringWithUnits function   *****************************
    // *****************************************************************************
    char* ValueToStringWithUnits(char* buf, uint32_t buf_size, int32_t val, int32_t scaler, const char* units);

  private:
    // Timer period
    static const uint32_t TASK_TIMER_PERIOD_MS = 1U;

    // Pointer to UART class
    StHalUart* uart = nullptr;

    // Buffer for transmit data
    uint8_t tx_buf[256u];
    // Buffer for receive data
    uint8_t rx_buf[512u];

    // Counter for received characters
    uint16_t rx_char_cnt = 0u;

    // Flag to show if we trying to gain control
    bool mpg_mode_request = true;

    // Command to request status. On MPG gain control request it would be replaced
    // request full status report command. Then it will reverted back.
    uint8_t status_request_command = CMD_STATUS_REPORT_LEGACY;
    // Flag show that status received after request. New status request will
    // not send until previous response received.
    bool status_received = true;
    // Flag show that we don't wait responses and can send another command.
    bool respond_pending = false;

    // When status last time was sent
    uint32_t status_tx_timestamp = 0u;
    // When status last time received
    uint32_t status_rx_timestamp = 0u;
    // When cmd was sent
    uint32_t cmd_tx_timestamp = 0u;
    // When ack last time received
    uint32_t cmd_rx_timestamp = 0u;

    // ID for next command
    uint32_t next_id = 1u;
    // ID of command that was send
    uint32_t send_id = 0u;

    // *************************************************************************
    // ***   GRBL Data   *******************************************************
    // *************************************************************************
    // Flags union
    typedef union
    {
      uint32_t flags;
      struct
      {
        uint32_t mpg : 1, state : 1, pos : 1, offset : 1, await_ack : 1,
                 await_wco_ok : 1, leds : 1, dist : 1, message : 1, feed : 1,
                 rpm : 1, alarm : 1, error : 1, xmode : 1, pins : 1, reset : 1,
                 feed_override : 1, rapid_override : 1, rpm_override : 1, probe: 1,
                 tlo: 1, unassigned : 11;
      };
    } changes_t;

    // GRBL state
    state_t   grbl_state;
    uint8_t   grbl_substate;
    float     grbl_position[AXIS_CNT];
    float     grbl_offset[AXIS_CNT];
    float     grbl_probe_position[AXIS_CNT];
    float     grbl_tool_length_offset[AXIS_CNT];
    int32_t   grbl_feed_override;
    int32_t   grbl_rapid_override;
    float     grbl_feed_rate;
    bool      grbl_useWPos;
    bool      grbl_awaitWCO;
    bool      grbl_absDistance;
    bool      grbl_mpgMode;
    bool      grbl_xModeDiameter;
    bool      grbl_probe_triggered = false;
    changes_t grbl_changed;
    changes_t grbl_received;
    uint8_t   grbl_alarm;
    status_t  grbl_status;
    char      grbl_pins[10];

    // Spindle state
    float   spindle_rpm_programmed;
    float   spindle_rpm_actual;
    bool    spindle_on;
    bool    spindle_ccw;
    int32_t spindle_rpm_override;

    // Coolant state
    bool coolant_flood;
    bool coolant_mist;

    // Flag to request settings
    bool request_settings = true;
    // Flag to show that settings changed
    bool settings_changed = false;

    // Number of axis
    int32_t number_of_axis = 3;

    // Settings
    uint8_t measurement_system = MEASUREMENT_SYSTEM_METRIC;
    uint8_t mode_of_operation = MODE_OF_OPERATION_MILL;
    uint16_t homing = 0u;
    uint16_t spindle_speed_max = 0u;
    uint16_t spindle_speed_min = 0u;

    // Task queue message struct
    struct TaskQueueMsg
    {
      uint32_t id;
      uint8_t cmd[128];
    };

    // Buffer for received task message
    TaskQueueMsg rcv_msg;

    // Mutex for synchronize when reads data
    RtosMutex mutex;

    // Axis definition. This array must match the Axis_t enum plus one for unknown axis
    char axis_str[AXIS_CNT + 1u][2u] = {"X", "Y", "Z", "A", "B", "C", "*"};

    // This array must match the grbl_state_t enum!
    const char* const grbl_state_str[STATE_CNT] =
    {"-----", "Idle", "Run", "Jog", "Hold", "Alarm", "Check", "Door", "Tool", "Home", "Sleep"};

    // *************************************************************************
    // ***   Alarm executor codes. Zero is reserved.   *************************
    // *************************************************************************
    // This array must match the alarm_code_t enum!
    const char* const grbl_alarm_str[Alarm_Code_Cnt + 1u] =
    {
      "None", "Hard Limit", "Soft Limit", "Abort Cycle", "Probe Fail Initial",
      "Probe Fail Contact", "Homing Fail Reset", "Homing Fail Door", "Fail Pulloff",
      "Homing Fail Approach", "EStop", "Homing Required", "Limits Engaged",
      "Probe Protect", "Spindle", "Homing Fail Auto Squaring Approach",
      "Selftest Failed", "Motor Fault", "Homing Fail", "AlarmCodeCnt"
    };

    // This array must match the status_t enum!
    const char* const grbl_status_str[Status_Status_Cnt + 1u] =
    {
     "OK", "Expected Command Letter", "Bad Number Format", "Invalid Statement", "Negative Value",
     "Homing Disabled", "Setting Step Pulse Min", "Setting Read Fail", "Idle Error", "System G Clock",
     "Soft Limit Error", "Overflow", "MaxStep Rate Exceeded", "Check Door", "Line Length Exceeded",
     "Travel Exceeded", "Invalid Jog Command", "Setting Disabled Laser", "Reset", "Non Positive Value",
     "Gcode Unsupported Command", "Gcode Modal Group Violation", "Gcode Undefined Feed Rate",
     "Gcode Command Value Not Integer", "Gcode Axis Command Conflict", "Gcode Word Repeated",
     "Gcode No Axis Words", "Gcode Invalid Line Number", "Gcode Value Word Missing",
     "Gcode Unsupported Coord Sys", "Gcode G53 Invalid Motion Mode", "Gcode Axis Words Exist",
     "Gcode No AxisWords In Plane", "Gcode Invalid Target", "Gcode Arc Radius Error",
     "Gcode No Offsets In Plane", "Gcode Unused Words", "Gcode G43 Dynamic Axis Error",
     "Gcode Illegal Tool Table Entry", "Gcode Value Out Of Range", "Gcode Tool Change Pending",
     "Gcode Spindle Not Running", "Gcode Illegal Plane", "Gcode Max Feed Rate Exceeded",
     "Gcode RPM Out Of Range", "Limits Engaged", "Homing Required", "GCode Tool Error",
     "Value Word Conflict", "Self Test Failed", "EStop", "Motor Fault", "Setting Value Out Of Range",
     "Setting Disabled", "Gcode Invalid Retract Position", "Illegal Homing Configuration",
     "GCode Coord System Locked",
     "", "", "", // Gap fillers between 56 and 60
     "SD Mount Error", "SD Read Error", "SD Failed Open Dir", "SD Dir NotFound", "SD File Empty",
     "", "", "", "", "", // Gap fillers between 64 and 70
     "BT Init Error", "Expression Uknown Op", "Expression Divide By Zero",
     "Expression Argument Out Of Range", "Expression Invalid Argument",
     "Expression Syntax Error", "Expression Invalid Result", "Authentication Required",
     "Access Denied", "Not Allowed Critical Event", "Flow Control Not Executing Macro",
     "Flow Control Syntax Error", "Flow Control Stack Overflow", "Flow Control Out Of Memory",
     "Next Cmd Executed", "Cmd Not Executed Yet", "Comm Error", "Unhandled", "StatusCnt"
    };


    // *************************************************************************
    // ***   Private: GetMeasurementSystemGcode function   *********************
    // *************************************************************************
    const char* GetMeasurementSystemGcode() {return (IsMetric() ? "G21" : "G20");}

    // *************************************************************************
    // ***   Private: ParseState function   ************************************
    // *************************************************************************
    bool ParseState(char* data);

    // *************************************************************************
    // ***   Private: ParseDecimal function   **********************************
    // *************************************************************************
    bool ParseDecimal(float& value, char* data);

    // *************************************************************************
    // ***   Private: ParseInt function   **************************************
    // *************************************************************************
    bool ParseInt(int32_t& value, char* data);

    // *************************************************************************
    // ***   Private: ParseSettings function   *********************************
    // *************************************************************************
    void ParseSettings(char* data);

    // *************************************************************************
    // ***   Private: ParseAxisData function   *********************************
    // *************************************************************************
    bool ParseAxisData(char* data, float (&axis)[AXIS_CNT]);

    // *************************************************************************
    // ***   Private: ParseOffsets function   **********************************
    // *************************************************************************
    void ParseOffsets(char* data);

    // *************************************************************************
    // ***   Private: ParseOverrides function   ********************************
    // *************************************************************************
    void ParseOverrides(char* data);

    // *************************************************************************
    // ***   Private: ParseFeedSpeed function   ********************************
    // *************************************************************************
    void ParseFeedSpeed(char* data);

    // *************************************************************************
    // ***   Private: ParseData function   *************************************
    // *************************************************************************
    void ParseData(void);

    // *************************************************************************
    // ***   Private: PollSerial function   ************************************
    // *************************************************************************
    void PollSerial(void);

    // *************************************************************************
    // ***   Private: GetNextId function   *************************************
    // *************************************************************************
    uint32_t GetNextId(void);

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    GrblComm() : AppTask(APPLICATION_TASK_STACK_SIZE, APPLICATION_TASK_PRIORITY,
                            "GrblComm", 32U, sizeof(TaskQueueMsg), &rcv_msg,
                            TASK_TIMER_PERIOD_MS, true) {};
};

#endif
