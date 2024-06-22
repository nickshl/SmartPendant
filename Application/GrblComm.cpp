//******************************************************************************
//  @file GrblComm.cpp
//  @author Nicolai Shlapunov
//
//  @details GrblComm: Grbl Communication Class, implementation
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

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "GrblComm.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

#if defined(SEND_DATA_TO_USB) // For sending messages to USB
#include "usb_device.h"
#include "usbd_cdc.h"
extern USBD_HandleTypeDef hUsbDeviceFS;
#endif

// *****************************************************************************
// ***   Public: Get Instance   ************************************************
// *****************************************************************************
GrblComm& GrblComm::GetInstance(void)
{
  static GrblComm grbl_comm;
  return grbl_comm;
}

// *****************************************************************************
// ***   Public: GrblComm Setup   **********************************************
// *****************************************************************************
Result GrblComm::Setup()
{
  // Init UART
  uart->Init();
  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Public: TimerExpired function   ***************************************
// *****************************************************************************
Result GrblComm::TimerExpired()
{
  // Process any received data
  PollSerial();

  // If status isn't received within 300 ms - something wrong.
  if(RtosTick::GetTimeMs() - status_rx_timestamp > 300u)
  {
    // Since there no status received, state is Unknown
    grbl_state = UNKNOWN;
    // Set status_received to request status again
    status_received = true;
  }

  // If we give up control or controller state is unknown, we should clear pending flag
  if((!IsInControl() || (grbl_state == UNKNOWN)) && respond_pending)
  {
    // If we lost control or if controller isn't responding - we don't expect answer anymore
    respond_pending = false;
    // Clear send id
    send_id = next_id;
    // Set appropriate error code for this situation
    grbl_status = Status_Comm_Error;
  }

  // If SW command used to gain control, MPG control requested and MPG isn't in control
  if(((NVM::GetInstance().GetCtrlTx() == CTRL_SW_COMMAND) || (NVM::GetInstance().GetCtrlTx() == CTRL_PIN_AND_SW_CMD)) && mpg_mode_request && !IsInControl())
  {
    // If mpg state received(not in control) or if last status was received a while ago
    if(grbl_received.mpg || (RtosTick::GetTimeMs() - status_rx_timestamp > 300u))
    {
      // Send command again
      SendRealTimeCmd(CMD_MPG_MODE_TOGGLE);
      // Clear status rx timestamp
      status_tx_timestamp = RtosTick::GetTimeMs();
      // Status received flag
      status_received = false;
      // Clear MPG state receive flag
      grbl_received.mpg = false;
    }
  }

  // If SmarpPendant is in control and request_settings flag is set
  if(IsInControl() && (request_settings))
  {
    // Request settings from the controller
    RequestControllerSettings();
    // And clear the request flag
    request_settings = false;
  }

  // Request status if previous one received and more than 100 ms passed since last request
  if(IsInControl() && status_received && (RtosTick::GetTimeMs() - status_tx_timestamp > 100u) && (uart->IsTxComplete()))
  {
    status_tx_timestamp = RtosTick::GetTimeMs();
    // Status received flag
    status_received = false;
    // Request status
    uart->Write((uint8_t*)"?", 1u);

#if defined(SEND_DATA_TO_USB)
    // Send to USB
    if(USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*)"?", 1u) == USBD_OK)
    {
      // Send packet - no waiting
      USBD_CDC_TransmitPacket(&hUsbDeviceFS);
    }
#endif
  }

  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Public: ProcessMessage function   *************************************
// *****************************************************************************
Result GrblComm::ProcessMessage()
{
  Result result = Result::ERR_BUSY;

  // We can write command only if there no ongoing transmission
  if(uart->IsTxComplete())
  {
    // If ID is zero - it is real time command(i.e. Run, Hold, Stop, etc.),
    // can be executed regardless who is in control
    if(rcv_msg.id == 0u)
    {
      // Copy command from message to transmit buffer
      strncpy((char*)tx_buf, (const char*)rcv_msg.cmd, NumberOf(tx_buf));
      // Send command
      result = uart->Write(tx_buf, strlen((char*)tx_buf));

#if defined(SEND_DATA_TO_USB)
      // Send to USB
      if(USBD_CDC_SetTxBuffer(&hUsbDeviceFS, tx_buf, strlen((char*)tx_buf)) == USBD_OK)
      {
        // Send packet - no waiting
        USBD_CDC_TransmitPacket(&hUsbDeviceFS);
      }
#endif
    }
    else if (IsInControl() && !respond_pending)
    {
      // If previous command successful
      if(grbl_status == Status_OK)
      {
        cmd_tx_timestamp = RtosTick::GetTimeMs();
        // Lock mutex before parsing data
        mutex.Lock();
        // Set pending flag
        respond_pending = true;
        // Set ID
        send_id = rcv_msg.id;
        // Release mutex after parsing data
        mutex.Release();
        // Copy command from message to transmit buffer
        strncpy((char*)tx_buf, (const char*)rcv_msg.cmd, NumberOf(tx_buf));
        // Send command
        result = uart->Write(tx_buf, strlen((char*)tx_buf));

#if defined(SEND_DATA_TO_USB)
        // Send to USB
        if(USBD_CDC_SetTxBuffer(&hUsbDeviceFS, tx_buf, strlen((char*)tx_buf)) == USBD_OK)
        {
          // Send packet - no waiting
          USBD_CDC_TransmitPacket(&hUsbDeviceFS);
        }
#endif
      }
      else
      {
        // Set ID
        send_id = rcv_msg.id;
        // Result ok, but not really
        result = Result::RESULT_OK;
      }
    }
    else
    {
      ; // Do nothing - MISRA rule
    }
  }

  // If message can't be processed right now
  if((result == Result::ERR_BUSY) || (result == Result::ERR_UART_BUSY))
  {
    // And if it is real time message or we have control
    if((rcv_msg.id == 0u) || IsInControl())
    {
      // Resend it to itself as priority since order is important
      result = SendTaskMessage(&rcv_msg, true);
      // And delay a little bit to allow other task to work
      RtosTick::DelayMs(TASK_TIMER_PERIOD_MS);
    }
  }
  else
  {
    // We can't send anything if MPG mode is off, so ignore it
    result = Result::RESULT_OK;
  }

  return result;
}

// *****************************************************************************
// ***   Public: GainControl function   ****************************************
// *****************************************************************************
void GrblComm::GainControl()
{
  if(NVM::GetInstance().GetCtrlTx() == CTRL_GPIO_PIN)
  {
    // Open drain
    HAL_GPIO_WritePin(MPG_EN_GPIO_Port, MPG_EN_Pin, GPIO_PIN_RESET);
  }
  else if(NVM::GetInstance().GetCtrlTx() == CTRL_SW_COMMAND)
  {
    // Send toggle command only if control wasn't requested before
    if(!mpg_mode_request)
    {
      SendRealTimeCmd(CMD_MPG_MODE_TOGGLE);
      // Set timestamp when status was received to track if we will get status
      // in response to CMD_MPG_MODE_TOGGLE command
      status_rx_timestamp = RtosTick::GetTimeMs();
    }
  }
  else if(NVM::GetInstance().GetCtrlTx() == CTRL_PIN_AND_SW_CMD)
  {
    // Open drain
    HAL_GPIO_WritePin(MPG_EN_GPIO_Port, MPG_EN_Pin, GPIO_PIN_RESET);
    // Send toggle command only if control wasn't requested before
    if(!mpg_mode_request)
    {
      SendRealTimeCmd(CMD_MPG_MODE_TOGGLE);
      // Set timestamp when status was received to track if we will get status
      // in response to CMD_MPG_MODE_TOGGLE command
      status_rx_timestamp = RtosTick::GetTimeMs();
    }
  }
  else
  {
    ; // Do nothing - MISRA rule
  }
  // Clear MPG state receive flag
  grbl_received.mpg = false;
  // Set flag to indicate we want to gain control
  mpg_mode_request = true;
}

// *****************************************************************************
// ***   Public: ReleaseControl function   *************************************
// *****************************************************************************
void GrblComm::ReleaseControl()
{
  if(NVM::GetInstance().GetCtrlTx() == CTRL_GPIO_PIN)
  {
    // Close drain
    HAL_GPIO_WritePin(MPG_EN_GPIO_Port, MPG_EN_Pin, GPIO_PIN_SET);
    // Set flag to indicate we want to give up control
    mpg_mode_request = false;
  }
  else if(NVM::GetInstance().GetCtrlTx() == CTRL_SW_COMMAND)
  {
    // Send toggle command only if control requested before and was granted
    if(mpg_mode_request && GetMpgMode())
    {
      SendRealTimeCmd(CMD_MPG_MODE_TOGGLE);
    }
    // Set flag to indicate we want to give up control
    mpg_mode_request = false;
  }
  else if(NVM::GetInstance().GetCtrlTx() == CTRL_PIN_AND_SW_CMD)
  {
    // Close drain
    HAL_GPIO_WritePin(MPG_EN_GPIO_Port, MPG_EN_Pin, GPIO_PIN_SET);
    // Send toggle command only if control requested before and was granted
    if(mpg_mode_request && GetMpgMode())
    {
      SendRealTimeCmd(CMD_MPG_MODE_TOGGLE);
    }
    // Set flag to indicate we want to give up control
    mpg_mode_request = false;
  }
  else if(NVM::GetInstance().GetCtrlTx() == CTRL_FULL)
  {
    // Never release control if Pendant in full control mode
  }
  else
  {
    ; // Should never get there
  }

  if(mpg_mode_request == false)
  {
    // Clear an error
    respond_pending = false;
    send_id = next_id;
    grbl_status = Status_OK;
    // Clear MPG state receive flag
    grbl_received.mpg = false;
  }
}

// *****************************************************************************
// ***   Public: SetFullControlMode function   *********************************
// *****************************************************************************
void GrblComm::SetFullControlMode(bool mode)
{
  // Set full control mode flag
  full_control = mode;
  // If full control mode is on
  if(full_control == true)
  {
    GainControl(); // Turn on TX line
  }
}

// *****************************************************************************
// ***   Public: GetAxisName function   ****************************************
// *****************************************************************************
const char* const GrblComm::GetAxisName(uint8_t axis)
{
  // Prevent out of boundaries
  if(axis > AXIS_CNT) axis = AXIS_CNT;
  // Return result
  return axis_str[axis];
}

// *****************************************************************************
// ***   Public: GetStateName function   ***************************************
// *****************************************************************************
const char* const GrblComm::GetStateName(state_t state)
{
  // Prevent out of boundaries
  if(state >= STATE_CNT)
  {
    state = UNKNOWN;
  }
  // Return result
  return grbl_state_str[state];
}

// *****************************************************************************
// ***   Public: GetStatusName function   **************************************
// *****************************************************************************
const char* const GrblComm::GetStatusName(status_t status)
{
  // Prevent out of boundaries
  if(status >= Status_Status_Cnt)
  {
    status = Status_Status_Cnt;
  }
  // Return result
  return grbl_status_str[status];
}

// *****************************************************************************
// ***   Public: GetAxisMachinePosition function   *****************************
// *****************************************************************************
int32_t GrblComm::GetAxisMachinePosition(uint8_t axis)
{
  // Return value - zero by default. Non existent axis always 0.
  int32_t value = 0;
  // Check parameter
  if(axis < AXIS_CNT)
  {
    // Lock mutex before copying data
    mutex.Lock();
    // Calculate value and convert it into fixed point(um for metric/tenths for imperial)
    value = (int32_t)(grbl_position[axis] * GetUnitsScaler());
    // Release mutex after data is copied
    mutex.Release();
  }
  // Return result
  return value;
}

// *****************************************************************************
// ***   Public: GetAxisPosition function   ************************************
// *****************************************************************************
int32_t GrblComm::GetAxisPosition(uint8_t axis)
{
  // Return value - zero by default. Non existent axis always 0.
  int32_t value = 0;
  // Check parameter
  if(axis < AXIS_CNT)
  {
    // Lock mutex before copying data
    mutex.Lock();
    // Calculate value and convert it into fixed point(um for metric/tenths for imperial)
    value = (int32_t)((grbl_position[axis] - grbl_offset[axis]) * GetUnitsScaler());
    // Release mutex after data is copied
    mutex.Release();
  }
  // Return result
  return value;
}

// *****************************************************************************
// ***   Public: GetProbeMachinePosition function   ****************************
// *****************************************************************************
int32_t GrblComm::GetProbeMachinePosition(uint8_t axis)
{
  // Return value - zero by default. Non existent axis always 0.
  int32_t value = 0;
  // Check parameter
  if(axis < AXIS_CNT)
  {
    // Lock mutex before copying data
    mutex.Lock();
    // Copy value
    value = (int32_t)(grbl_probe_position[axis] * GetUnitsScaler());
    // Release mutex after data is copied
    mutex.Release();
  }
  // Return result
  return value;
}

// *****************************************************************************
// ***   Public: GetProbePosition function   ***********************************
// *****************************************************************************
int32_t GrblComm::GetProbePosition(uint8_t axis)
{
  // Return value - zero by default. Non existent axis always 0.
  int32_t value = 0;
  // Check parameter
  if(axis < AXIS_CNT)
  {
    // Lock mutex before copying data
    mutex.Lock();
    // Copy value
    value = (int32_t)((grbl_probe_position[axis] - grbl_offset[axis]) * GetUnitsScaler());
    // Release mutex after data is copied
    mutex.Release();
  }
  // Return result
  return value;
}

// *****************************************************************************
// ***   Public: GetCmdResult   ************************************************
// *****************************************************************************
GrblComm::status_t GrblComm::GetCmdResult(uint32_t id)
{
  status_t status = Status_Cmd_Not_Executed_Yet;

  // If respond received and last send ID match requested
  if((respond_pending == false) && (id == send_id))
  {
    // Use current status
    status = grbl_status;
  }
  else
  {
    // If requested ID less than we already sent - status is lost
    if(id < send_id)
    {
      status = Status_Next_Cmd_Executed;
    }
  }

  return status;
}

// *****************************************************************************
// ***   Public: SendCmd   *****************************************************
// *****************************************************************************
Result GrblComm::SendCmd(const char* cmd, uint32_t &id)
{
  Result result = Result::ERR_CANNOT_EXECUTE;

  // We able to send command only if we in control and in known state
  if(IsInControl() && (grbl_state != UNKNOWN))
  {
    TaskQueueMsg msg;

    // Set ID for command
    msg.id = GetNextId();
    // Cycle to copy command
    for(uint32_t i = 0u; i < NumberOf(msg.cmd); i++)
    {
      // Copy one byte
      msg.cmd[i] = cmd[i];
      // Check if it is null-terminator
      if(msg.cmd[i] == '\0')
      {
        // Set good result
        result = Result::RESULT_OK;
        // And break the cycle
        break;
      }
    }

    // If we able to copy command
    if(result.IsGood())
    {
      // Send the message
      result = SendTaskMessage(&msg);
      // Save ID
      id = msg.id;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: SendRealTimeCmd   *********************************************
// *****************************************************************************
Result GrblComm::SendRealTimeCmd(uint8_t cmd)
{
  // Real time commands can be send any time
  TaskQueueMsg msg;
  // Set ID to 0 - real time command
  msg.id = 0u;
  // Set real time command and null-terminator
  msg.cmd[0] = cmd;
  msg.cmd[1] = '\0';

  // Send the message as priority since it is real-time command
  return SendTaskMessage(&msg, true);
}

// *****************************************************************************
// ***   Public: Unlock   ******************************************************
// *****************************************************************************
Result GrblComm::Unlock()
{
  // Set Ok status to allow process unlock command
  grbl_status = Status_OK;
  // Command ID
  uint32_t id = 0u;
  // Command buffer
  static const char cmd[] = {"$X\r\n"};
  // Send command
  return SendCmd(cmd, id);
}

// *****************************************************************************
// ***   Public: RequestControllerSettings   ***********************************
// *****************************************************************************
Result GrblComm::RequestControllerSettings()
{
  // Command ID
  uint32_t id = 0u;
  // Command buffer
  static const char cmd[] = {"$$\r\n"};
  // Send command
  return SendCmd(cmd, id);
}

// *****************************************************************************
// ***   Public: RequestOffsets   **********************************************
// *****************************************************************************
Result GrblComm::RequestOffsets()
{
  // Command ID
  uint32_t id = 0u;
  // Command buffer
  static const char cmd[] = {"$#\r\n"};
  // Send command
  return SendCmd(cmd, id);
}

// *****************************************************************************
// ***   Public: Jog   *********************************************************
// *****************************************************************************
Result GrblComm::Jog(uint8_t axis, int32_t distance, uint32_t speed_x100, bool is_absolute)
{
  Result result = Result::ERR_BAD_PARAMETER;

  if(axis < AXIS_CNT)
  {
    // Jogging possible only in Idle or Jog states
    if(IsInControl() && ((grbl_state == IDLE) || (grbl_state == JOG)))
    {
      TaskQueueMsg msg;

      // Set ID for command
      msg.id = GetNextId();

      // Buffers for distance and speed strings
      char distance_str[16u];
      char speed_str[16u];
      // Convert distance & speed values to strings
      ValueToString(distance_str, NumberOf(distance_str), distance, GetUnitsScaler());
      ValueToString(speed_str, NumberOf(speed_str), speed_x100, 100);
      // Create jog string
      snprintf((char*)msg.cmd, NumberOf(msg.cmd), "$J=%s%s%s%sF%s\r\n", GetMeasurementSystemGcode(), is_absolute ? "G90" : "G91", axis_str[axis], distance_str, speed_str);

      // Send message
      result = SendTaskMessage(&msg);
    }
    else
    {
      result = Result::ERR_CANNOT_EXECUTE;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: JogInMachineCoodinates   **************************************
// *****************************************************************************
Result GrblComm::JogInMachineCoodinates(uint8_t axis, int32_t distance, uint32_t speed)
{
  Result result = Result::ERR_BAD_PARAMETER;

  if(axis < AXIS_CNT)
  {
    // Jogging possible only in Idle or Jog states
    if(IsInControl() && ((grbl_state == IDLE) || (grbl_state == JOG)))
    {
      TaskQueueMsg msg;

      // Set ID for command
      msg.id = GetNextId();

      // Buffer for distance string
      char distance_str[16u];
      // Convert distance value to string
      ValueToString(distance_str, NumberOf(distance_str), distance, GetUnitsScaler());
      // Create jog string
      snprintf((char*)msg.cmd, NumberOf(msg.cmd), "$J=%sG53G90%s%sF%lu\r\n", GetMeasurementSystemGcode(), axis_str[axis], distance_str, speed);

      // Send message
      result = SendTaskMessage(&msg);
    }
    else
    {
      result = Result::ERR_CANNOT_EXECUTE;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: JogMultiple   *************************************************
// *****************************************************************************
Result GrblComm::JogMultiple(int32_t distance_x, int32_t distance_y, int32_t distance_z, uint32_t speed_x100, bool is_absolute)
{
  Result result = Result::RESULT_OK;

  // Jogging possible only in Idle or Jog states
  if(IsInControl() && ((grbl_state == IDLE) || (grbl_state == JOG)))
  {
    TaskQueueMsg msg;

    // Set ID for command
    msg.id = GetNextId();

    // Buffer for distance string
    char distance_x_str[16u];
    char distance_y_str[16u];
    char distance_z_str[16u];
    char speed_str[16u];
    // Convert distance value to string
    ValueToString(distance_x_str, NumberOf(distance_x_str), distance_x, GetUnitsScaler());
    ValueToString(distance_y_str, NumberOf(distance_y_str), distance_y, GetUnitsScaler());
    ValueToString(distance_z_str, NumberOf(distance_z_str), distance_z, GetUnitsScaler());
    ValueToString(speed_str, NumberOf(speed_str), speed_x100, 100);
    // Create jog string
    snprintf((char*)msg.cmd, NumberOf(msg.cmd), "$J=%s%s%s%s%s%s%s%sF%s\r\n", GetMeasurementSystemGcode(), is_absolute ? "G90" : "G91",
                                                axis_str[AXIS_X], distance_x_str, axis_str[AXIS_Y], distance_y_str,
                                                axis_str[AXIS_Z], distance_z_str, speed_str);

    // Send message
    result = SendTaskMessage(&msg);
  }
  else
  {
    result = Result::ERR_CANNOT_EXECUTE;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: JogArcXYR   ***************************************************
// *****************************************************************************
Result GrblComm::JogArcXYR(int32_t x, int32_t y, uint32_t r, uint32_t speed_x100, bool direction, bool is_absolute)
{
  Result result = Result::RESULT_OK;

  static int32_t x_prev = 0u;
  static int32_t y_prev = 0u;

  if((x_prev == x) && (y_prev == y))
  {
    x_prev = x;
    y_prev = y;
  }

  x_prev = x;
  y_prev = y;

  // Jogging possible only in Idle or Jog states
  if(IsInControl() && ((grbl_state == IDLE) || (grbl_state == JOG) || (grbl_state == RUN)))
  {
    TaskQueueMsg msg;

    // Set ID for command
    msg.id = GetNextId();

    // Buffers for distance and speed strings
    char x_str[16u];
    char y_str[16u];
    char r_str[16u];
    char speed_str[16u];
    // Convert distance & speed values to strings
    ValueToString(x_str, NumberOf(x_str), x, GetUnitsScaler());
    ValueToString(y_str, NumberOf(y_str), y, GetUnitsScaler());
    ValueToString(r_str, NumberOf(r_str), r, GetUnitsScaler());
    ValueToString(speed_str, NumberOf(speed_str), speed_x100, 100);

    // Create Jog command
    snprintf((char*)msg.cmd, NumberOf(msg.cmd), "%sG17%s%s%s%s%s%sR%sF%s\r\n", GetMeasurementSystemGcode(),
                                                is_absolute ? "G90" : "G91", direction ? "G02" : "G03",
                                                axis_str[AXIS_X], x_str, axis_str[AXIS_Y], y_str, r_str, speed_str);

    // Send message
    result = SendTaskMessage(&msg);
  }
  else
  {
    result = Result::ERR_CANNOT_EXECUTE;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: ZeroAxis   ****************************************************
// *****************************************************************************
Result GrblComm::ZeroAxis(uint8_t axis)
{
  Result result = Result::ERR_BAD_PARAMETER;

  if(axis < AXIS_CNT)
  {
    // Reset axis possible only in Idle state
    if(IsInControl() && (grbl_state == IDLE))
    {
      TaskQueueMsg msg;

      // Set ID for command
      msg.id = GetNextId();
      // Create Zero Axis command
      snprintf((char*)msg.cmd, NumberOf(msg.cmd), "G90G10L20P0%s0\r\n", axis_str[axis]); // G90G10L20P0X0

      // Send message
      result = SendTaskMessage(&msg);
    }
    else
    {
      result = Result::ERR_CANNOT_EXECUTE;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: SetAxis   *****************************************************
// *****************************************************************************
Result GrblComm::SetAxis(uint8_t axis, int32_t position)
{
  Result result = Result::ERR_BAD_PARAMETER;

  if(axis < AXIS_CNT)
  {
    // Reset axis possible only in Idle state
    if(IsInControl() && (grbl_state == IDLE))
    {
      TaskQueueMsg msg;

      // Set ID for command
      msg.id = GetNextId();

      // Buffer for distance string
      char position_str[16u];
      // Convert distance value to string
      ValueToString(position_str, NumberOf(position_str), position, GetUnitsScaler());
      // Create Set Axis command
      snprintf((char*)msg.cmd, NumberOf(msg.cmd), "%sG90G10L20P0%s%s\r\n", GetMeasurementSystemGcode(), axis_str[axis], position_str);

      // Send message
      result = SendTaskMessage(&msg);
    }
    else
    {
      result = Result::ERR_CANNOT_EXECUTE;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: SetAbsoluteMode   *********************************************
// *****************************************************************************
Result GrblComm::SetAbsoluteMode()
{
  // Command ID
  uint32_t id = 0u;
  // Command buffer
  static const char cmd[] = {"G90\r\n"};
  // Send command
  return SendCmd(cmd, id);
}

// *****************************************************************************
// ***   Public: SetIncrementalMode   ******************************************
// *****************************************************************************
Result GrblComm::SetIncrementalMode()
{
  // Command ID
  uint32_t id = 0u;
  // Command buffer
  static const char cmd[] = {"G91\r\n"};
  // Send command
  return SendCmd(cmd, id);
}

// *****************************************************************************
// ***   Public: SetLatheRadiusMode   ******************************************
// *****************************************************************************
Result GrblComm::SetLatheRadiusMode()
{
  // Command ID
  uint32_t id = 0u;
  // Command buffer
  static const char cmd[] = {"G8\r\n"};
  // Send command
  return SendCmd(cmd, id);
}

// *****************************************************************************
// ***   Public: SetLatheDiameterMode   ****************************************
// *****************************************************************************
Result GrblComm::SetLatheDiameterMode()
{
  // Command ID
  uint32_t id = 0u;
  // Command buffer
  static const char cmd[] = {"G7\r\n"};
  // Send command
  return SendCmd(cmd, id);
}

// *****************************************************************************
// ***   Public: MoveAxis   ****************************************************
// *****************************************************************************
Result GrblComm::MoveAxis(uint8_t axis, int32_t distance, uint32_t speed_x100, uint32_t &id)
{
  Result result = Result::ERR_BAD_PARAMETER;

  if(axis < AXIS_CNT)
  {
    // Move possible only in Idle state
    if(IsInControl() && (grbl_state == IDLE))
    {
      TaskQueueMsg msg;

      // Set ID for command
      msg.id = GetNextId();

      // Buffers for distance and speed strings
      char distance_str[16u];
      char speed_str[16u];
      // Convert distance & speed values to strings
      ValueToString(distance_str, NumberOf(distance_str), distance, GetUnitsScaler());
      ValueToString(speed_str, NumberOf(speed_str), speed_x100, 100);

      // Create move command(zero speed mean rapid)
      if(speed_x100)
      {
        snprintf((char*)msg.cmd, NumberOf(msg.cmd), "%sG1%s%sF%s\r\n", GetMeasurementSystemGcode(), axis_str[axis], distance_str, speed_str);
      }
      else
      {
        snprintf((char*)msg.cmd, NumberOf(msg.cmd), "%sG0%s%s\r\n", GetMeasurementSystemGcode(), axis_str[axis], distance_str);
      }

      // Send message
      result = SendTaskMessage(&msg);
      // Save ID
      id = msg.id;
    }
    else
    {
      result = Result::ERR_CANNOT_EXECUTE;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: ProbeAxisTowardWorkpiece   ************************************
// *****************************************************************************
Result GrblComm::ProbeAxisTowardWorkpiece(uint8_t axis, int32_t position, uint32_t speed, uint32_t &id)
{
  Result result = Result::ERR_BAD_PARAMETER;

  if(axis < AXIS_CNT)
  {
    // Reset axis possible only in Idle state
    if(IsInControl() && (grbl_state == IDLE))
    {
      TaskQueueMsg msg;

      // Set ID for command
      msg.id = GetNextId();

      // Buffer for distance string
      char position_str[16u];
      // Convert distance value to string
      ValueToString(position_str, NumberOf(position_str), position, GetUnitsScaler());
      // Create Set Axis command
      snprintf((char*)msg.cmd, NumberOf(msg.cmd), "%sG38.2%s%sF%lu\r\n", GetMeasurementSystemGcode(), axis_str[axis], position_str, speed);

      // Send message
      result = SendTaskMessage(&msg);
      // Save ID
      id = msg.id;
    }
    else
    {
      result = Result::ERR_CANNOT_EXECUTE;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: ProbeAxisAwayFromWorkpiece   **********************************
// *****************************************************************************
Result GrblComm::ProbeAxisAwayFromWorkpiece(uint8_t axis, int32_t position, uint32_t speed, uint32_t &id)
{
  Result result = Result::ERR_BAD_PARAMETER;

  if(axis < AXIS_CNT)
  {
    // Reset axis possible only in Idle state
    if(IsInControl() && (grbl_state == IDLE))
    {
      TaskQueueMsg msg;

      // Set ID for command
      msg.id = GetNextId();

      // Buffer for distance string
      char position_str[16u];
      // Convert distance value to string
      ValueToString(position_str, NumberOf(position_str), position, GetUnitsScaler());
      // Create Set Axis command
      snprintf((char*)msg.cmd, NumberOf(msg.cmd), "%sG38.4%s%sF%lu\r\n", GetMeasurementSystemGcode(), axis_str[axis], position_str, speed);

      // Send message
      result = SendTaskMessage(&msg);
      // Save ID
      id = msg.id;
    }
    else
    {
      result = Result::ERR_CANNOT_EXECUTE;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: SetToolLengthOffset   *****************************************
// *****************************************************************************
Result GrblComm::SetToolLengthOffset(int32_t offset)
{
  Result result = Result::ERR_BAD_PARAMETER;

  // Reset axis possible only in Idle state
  if(IsInControl() && (grbl_state == IDLE))
  {
    TaskQueueMsg msg;
    // Set ID for command
    msg.id = GetNextId();

    // Buffer for distance string
    char offset_str[16u];
    // Convert distance value to string
    ValueToString(offset_str, NumberOf(offset_str), offset, GetUnitsScaler());
    // Create Set Axis command
    snprintf((char*)msg.cmd, NumberOf(msg.cmd), "%sG43.1Z%s\r\n", GetMeasurementSystemGcode(), offset_str);

    // Send message
    result = SendTaskMessage(&msg);
  }
  else
  {
    result = Result::ERR_CANNOT_EXECUTE;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: ClearToolLengthOffset   ***************************************
// *****************************************************************************
Result GrblComm::ClearToolLengthOffset()
{
  Result result = Result::ERR_BAD_PARAMETER;

  // Reset axis possible only in Idle state
  if(IsInControl() && (grbl_state == IDLE))
  {
    TaskQueueMsg msg;
    // Set ID for command
    msg.id = GetNextId();
    // Create Set Axis command
    snprintf((char*)msg.cmd, NumberOf(msg.cmd), "G49\r\n");
    // Send message
    result = SendTaskMessage(&msg);
  }
  else
  {
    result = Result::ERR_CANNOT_EXECUTE;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: ValueToString function   **************************************
// *****************************************************************************
char* GrblComm::ValueToString(char* buf, uint32_t buf_size, int32_t val, int32_t scaler)
{
  // Find sign: sigh should be handled separately, because it will be lost
  // for values less than scaler.
  bool is_negative = val < 0;
  // Remove sign from number
  val = abs(val);

  // Check scaler and use appropriate format
  if(scaler == 10000)
  {
    snprintf(buf, buf_size, "%s%lu.%04lu", is_negative ? "-" : "", val / scaler, val % scaler);
  }
  else if(scaler == 1000)
  {
    snprintf(buf, buf_size, "%s%lu.%03lu", is_negative ? "-" : "", val / scaler, val % scaler);
  }
  else if(scaler == 100)
  {
    snprintf(buf, buf_size, "%s%lu.%02lu", is_negative ? "-" : "", val / scaler, val % scaler);
  }
  else if(scaler == 10)
  {
    snprintf(buf, buf_size, "%s%lu.%01lu", is_negative ? "-" : "", val / scaler, val % scaler);
  }
  else
  {
    snprintf(buf, buf_size, "%s%lu", is_negative ? "-" : "", val);
  }

  return buf;
}

// *****************************************************************************
// ***   Private: ParseState function   ****************************************
// *****************************************************************************
bool GrblComm::ParseState(char *data)
{
  uint8_t state = 0u;
  uint8_t substate = 0u;
  bool changed = false;
  char *s = strchr(data, ':');

  // If substate exist
  if(s)
  {
    // Replace ':' with null-terminator
    *s++ = '\0';
    // And convert substate to number
    substate = atoi(s);
  }

  // Check all sates
  while((state < STATE_CNT) && strcmp(data, grbl_state_str[state]))
  {
    state++;
  }

  // If state isn't found - set it to Unknown
  if(state >= STATE_CNT)
  {
    state = UNKNOWN;
  }

  // If state found and changed
  if((state < STATE_CNT) && ((grbl_state != state) || (grbl_substate != substate)))
  {
    // If previous state was UNKNOWN, set flag to request controller settings
    if(grbl_state == UNKNOWN)
    {
      request_settings = true;
    }

    // Save new state and set changed flag
    grbl_state = (state_t)state;
    grbl_substate = substate;
    changed = true;
  }

  return changed;
}

// *****************************************************************************
// ***   Private: ParseDecimal function   **************************************
// *****************************************************************************
bool GrblComm::ParseDecimal(float& value, char* data)
{
  // Convert float from string
  float val = (float)atof(data);
  // Check if it changed
  bool changed = (val != value);
  // If changed - set new value
  if(changed) value = val;
  // Return status
  return changed;
}

// *****************************************************************************
// ***   Private: ParseInt function   ******************************************
// *****************************************************************************
bool GrblComm::ParseInt(int32_t& value, char* data)
{
  // Convert int from string
  int32_t val = (int32_t)atoi(data);
  // Check if it changed
  bool changed = (val != value);
  // If changed - set new value
  if(changed) value = val;
  // Return status
  return changed;
}

// *****************************************************************************
// ***   Private: ParseAxisData function   *************************************
// *****************************************************************************
bool GrblComm::ParseAxisData(char* data, float (&axis)[AXIS_CNT])
{
  bool changed = false;

  // Cycle for all axis
  for(uint32_t i = 0u; i < AXIS_CNT; i++)
  {
    // Parse probe value
    if(ParseDecimal(axis[i], data)) changed = true;

    // Find next comma
    char* next = strchr(data, ',');
    // If it found
    if(next)
    {
      // Update data pointer to next after comma character
      data = next + 1u;
    }
    else // No more comma found
    {
      break; // the cycle
    }
  }

  return changed;
}

// *****************************************************************************
// ***   Private: ParseOffsets function   **************************************
// *****************************************************************************
void GrblComm::ParseOffsets(char* data)
{
  if(grbl_useWPos)
  {
    grbl_changed.offset = (grbl_offset[AXIS_X] != 0.0f) || (grbl_offset[AXIS_Y] != 0.0f) || (grbl_offset[AXIS_Z] != 0.0f);

    grbl_offset[AXIS_X] = 0.0f;
    grbl_offset[AXIS_Y] = 0.0f;
    grbl_offset[AXIS_Z] = 0.0f;
  }
  else
  {
    grbl_changed.offset = ParseAxisData(data, grbl_offset);
  }

  grbl_changed.await_wco_ok = grbl_awaitWCO;
}

// *****************************************************************************
// ***   Private: ParseOverrides function   ************************************
// *****************************************************************************
void GrblComm::ParseOverrides(char* data)
{
  char* next = nullptr;

  next = strchr(data, ','); // Find next comma
  *next++ = '\0';           // And replace it with zero

  // Parse feed override
  if(ParseInt(grbl_feed_override, data)) grbl_changed.feed_override = true;

  data = next;              // Update data pointer
  next = strchr(data, ','); // Find next comma
  *next++ = '\0';           // And replace it with zero

  // Parse rapid override
  if(ParseInt(grbl_rapid_override, data)) grbl_changed.rapid_override = true;
  // Parse RPM override
  if(ParseInt(spindle_rpm_override, next)) grbl_changed.rpm_override = true;
}

// *****************************************************************************
// ***   Private: ParseFeedSpeed function   ************************************
// *****************************************************************************
void GrblComm::ParseFeedSpeed(char* data)
{
  char* next = nullptr;

  next = strchr(data, ','); // Find next comma
  *next++ = '\0';           // And replace it with zero

  // Parse feed rate
  if(ParseDecimal(grbl_feed_rate, data)) grbl_changed.feed = true;

  data = next; // Update data pointer
  // Try to find next comma
  if((next = strchr(data, ','))) *next++ = '\0';

  // Parse spindle programmed rpm
  if(ParseDecimal(spindle_rpm_programmed, data)) grbl_changed.rpm = true;
  // Parse spindle actual rpm
  if(next && ParseDecimal(spindle_rpm_actual, next)) grbl_changed.rpm = true;
}

// *****************************************************************************
// ***   Private: ParseData function   *****************************************
// *****************************************************************************
void GrblComm::ParseData(void)
{
  uint32_t c;
  bool pins = true;
  char* line = (char*)&rx_buf[0];

  // Check "ok" response
  if(!strcmp((char*)rx_buf, "ok"))
  {
    respond_pending = false;
    grbl_status = Status_OK;
    return;
  }

  // Parse settings
  if(line[0] == '$')
  {
    char *s = strchr(line, '=');
    // If  '=' sign found
    if(s)
    {
      // Replace '=' with null-terminator
      *s++ = '\0';

      // Find settings number
      uint32_t setting_num = atoi(&line[1]);

      // Find setting and store it
      switch(setting_num)
      {
        // *********************************************************************
        case 13:
          if(measurement_system != atoi(s))
          {
            measurement_system = atoi(s);
            settings_changed = true;
          }
          break;

        // *********************************************************************
        case 32:
          if(mode_of_operation != atoi(s))
          {
            mode_of_operation = atoi(s);
            settings_changed = true;
          }
          break;

        // *********************************************************************
        default:
          break;
      }
    }
  }

  // Parse status
  if(line[0] == '<')
  {
    // Set status received flag
    status_received = true;
    // Set timestamp when status was received
    status_rx_timestamp = RtosTick::GetTimeMs();

    pins = false;
    line = strtok(&line[1], "|");

    if(line)
    {
      if(ParseState(line))
      {
        grbl_changed.state = true;
//        if(!(grbl_state == ALARM || grbl_state == TOOL) && grbl_message[0] != '\0') grblClearMessage();
      }
      if(grbl_alarm && grbl_state != ALARM)
      {
        grbl_alarm = 0u;
        grbl_changed.alarm = false;
      }
      line = strtok(NULL, "|");
    }

    while(line)
    {
      if(!strncmp(line, "WPos:", 5))
      {
        if(!grbl_useWPos)
        {
          grbl_useWPos = true;
          grbl_changed.offset = true;
        }
        grbl_changed.pos = ParseAxisData(line + 5, grbl_position);
      }
      else if(!strncmp(line, "MPos:", 5))
      {
        if(grbl_useWPos)
        {
          grbl_useWPos = false;
          grbl_changed.offset = true;
        }
        grbl_changed.pos = ParseAxisData(line + 5, grbl_position);
      }
      else if(!strncmp(line, "FS:", 3)) ParseFeedSpeed(line + 3);

      else if(!strncmp(line, "WCO:", 4)) ParseOffsets(line + 4);

      else if(!strncmp(line, "Pn:", 3))
      {
        pins = true;
        if((grbl_changed.pins = (strcmp(grbl_pins, line + 3) != 0)));
        strcpy(grbl_pins, line + 3);
      }
      else if(!strncmp(line, "D:", 2))
      {
        grbl_xModeDiameter = line[2] == '1';
        grbl_changed.xmode = true;
      }
      else if(!strncmp(line, "A:", 2))
      {
        line = &line[2];
        spindle_on = spindle_ccw = coolant_flood = coolant_mist = false;
        grbl_changed.leds = true;

        while((c = *line++))
        {
          switch(c)
          {
            case 'M':
              coolant_mist = true;
              break;

            case 'F':
              coolant_flood = true;
              break;

            case 'S':
              spindle_ccw = false;
              spindle_on = true;
              break;

            case 'C':
              spindle_ccw = true;
              spindle_on = true;
              break;
          }
        }
      }
      else if(!strncmp(line, "Ov:", 3)) ParseOverrides(line + 3);

      else if(!strncmp(line, "MPG:", 4))
      {
        if(grbl_mpgMode != (line[4] == '1'))
        {
          grbl_mpgMode = !grbl_mpgMode;
          grbl_changed.mpg = true;
//          grbl_event.on_line_received = parseData;
        }
        grbl_received.mpg = true;
      }
      else if(!strncmp(line, "SD:", 3))
      {
//        if((grbl_changed.message = !!strcmp(grbl_message, line + 3))) strncpy(grbl_message, line + 3, 250);
      }

      line = strtok(NULL, "|");
    }

    if(!pins && (grbl_changed.pins = (grbl_pins[0] != '\0'))) grbl_pins[0] = '\0';
  }
  else if(line[0] == '[')
  {
    if(!strncmp(&line[1], "PRB:", 4))
    {
      // Probe position
      grbl_changed.probe = ParseAxisData(line + 1 + 4, grbl_probe_position);
    }
    else if(!strncmp(&line[1], "TLO:", 4))
    {
      // Tool Length Offset
      grbl_changed.tlo = ParseAxisData(line + 1 + 4, grbl_tool_length_offset);
    }
  }
  else if(!strncmp(line, "error:", 6))
  {
    grbl_status = (status_t)atoi(line + 6);
    grbl_changed.error = true;
    respond_pending = false;
  }
  else if(!strncmp(line, "ALARM:", 6))
  {
    grbl_alarm = (uint8_t)atoi(line + 6);
    grbl_changed.alarm = true;
  }
}

// *****************************************************************************
// ***   Private: PollSerial function   ****************************************
// *****************************************************************************
void GrblComm::PollSerial(void)
{
  uint8_t c = 0u;

  while(uart->Read(c) == Result::RESULT_OK)
  {
    if(c == 0x18u) // ASCII_CAN
    {
      rx_char_cnt = 0u;
    }
    else if(((c == '\n') || (c == '\r')))
    {
      // If we have at least one character
      if(rx_char_cnt > 0u)
      {
#if defined(SEND_DATA_TO_USB)
        // Add CR LF and null-terminator for USB
        rx_buf[rx_char_cnt + 0u] = '\n';
        rx_buf[rx_char_cnt + 1u] = '\r';
        rx_buf[rx_char_cnt + 2u] = '\0';
        // Send to USB
        if(USBD_CDC_SetTxBuffer(&hUsbDeviceFS, rx_buf, strlen((char*)rx_buf)) == USBD_OK)
        {
          // Send packet - no waiting
          USBD_CDC_TransmitPacket(&hUsbDeviceFS);
        }
#endif
        // End of line reached
        rx_buf[rx_char_cnt] = '\0';

        // Lock mutex before parsing data
        mutex.Lock();
        // Try to parse it
        ParseData();
        // Release mutex after parsing data
        mutex.Release();
      }
      rx_char_cnt = 0u;
    }
    else if(rx_char_cnt < NumberOf(rx_buf) - 1u - 2u) // minus one for null-terminator, minus 2 for \n\r for USB debug
    {
      rx_buf[rx_char_cnt++] = (char)c;
    }
  }
}

// *****************************************************************************
// ***   Private: GetNextId function   *****************************************
// *****************************************************************************
uint32_t GrblComm::GetNextId(void)
{
  // Lock mutex before parsing data
  mutex.Lock();
  // get next ID
  uint32_t nid = next_id;
  // And increase it
  next_id++;
  // Check zero - isn't valid ID(I doubt any will stream 4'294'967'295 commands, but...)
  if(next_id == 0u) next_id++;
  // Release mutex after parsing data
  mutex.Release();
  // Return result
  return nid;
}
