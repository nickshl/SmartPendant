//******************************************************************************
//  @file InputDrv.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: Input Driver Class, implementation
//
//  @copyright Copyright (c) 2016, Devtronic & Nicolai Shlapunov
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
#include "InputDrv.h"

#if defined(INPUTDRV_ENABLED)

#include "Rtos.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
InputDrv& InputDrv::GetInstance(void)
{
  // This class is static and declared here
  static InputDrv input_drv;
  // Return reference to class
  return input_drv;
}

// *****************************************************************************
// ***   Init Input Driver Task   **********************************************
// *****************************************************************************
void InputDrv::InitTask(TIM_HandleTypeDef* htm, ADC_HandleTypeDef* had)
{
  // Save timer handle
  htim = htm;
  // Save ADC handle
  hadc = had;
  // Auto detect left input device
  devices[EXT_LEFT] = DetectDeviceType(EXT_LEFT);
  // Auto detect right input device
  devices[EXT_RIGHT] = DetectDeviceType(EXT_RIGHT);

  // Configure ADC
  ConfigADC(devices[EXT_LEFT], devices[EXT_RIGHT]);

  // Create task
  CreateTask();
}

// *****************************************************************************
// ***   Input Driver Setup   **************************************************
// *****************************************************************************
Result InputDrv::Setup()
{
  // If has timer handle and at least one device is encoder
  if(   (htim != nullptr)
     && ((devices[EXT_LEFT] == EXT_DEV_ENC) || (devices[EXT_RIGHT] == EXT_DEV_ENC)))
  {
    // Start timer for get interrupts. Should be there, because FreeRTOS
    // must initialize tasks stacks before runs interrupt.
    HAL_TIM_Base_Start_IT(htim);
  }
  // Init ticks variable
  last_wake_ticks = RtosTick::GetTickCount();
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Input Driver Loop   ***************************************************
// *****************************************************************************
Result InputDrv::Loop()
{
  // Call interrupt handler
  ProcessInput();
  // Pause until next tick
  RtosTick::DelayUntilMs(last_wake_ticks, 1U);
  // Always run
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Process Input function   **********************************************
// *****************************************************************************
void InputDrv::ProcessInput(void)
{
  // Cycle for process devices
  for(uint32_t i = 0U; i < EXT_MAX; i++)
  {
    // Check device type
    switch(devices[i])
    {
      // Process buttons device
      case EXT_DEV_BTN:
        // Process all buttons
        for(uint32_t j = 0U; j < BTN_MAX; j++)
        {
          ProcessButtonInput(buttons[i].button[j]);
        }
        // Use buttons to set state of "virtual" encoder
        EmulateEncodersByButtons((PortType)i);
        break;

      // Process encoder device
      case EXT_DEV_ENC:
        // If no timer handle
        if(htim == nullptr)
        {
          // Process encoders input in task function
          ProcessEncoderInput(encoders[i].enc);
        }
        // Always process encoder buttons in task function
        ProcessButtonInput(encoders[i].btn[ENC_BTN_ENT]);
        ProcessButtonInput(encoders[i].btn[ENC_BTN_BACK]);
        break;

      // Process joysticks device
      case EXT_DEV_JOY:
        // Process all joysticks
        ProcessJoystickInput(joysticks[i].joy, (PortType)i);
        // Process joystick button
        ProcessButtonInput(joysticks[i].btn);
        // Start measurement
        HAL_ADCEx_InjectedStart(hadc);
        // Use joystick to set state of "virtual" buttons
        EmulateButtonsByJoystick((PortType)i);
        // Use buttons to set state of "virtual" encoder
        EmulateEncodersByButtons((PortType)i);
        break;

      case EXT_DEV_NONE:
      case EXT_DEV_MAX:
      default:
        break;
    }
  }
}

// *****************************************************************************
// ***   Process Button Input function   ***************************************
// *****************************************************************************
void InputDrv::ProcessButtonInput(ButtonProfile& button)
{
  // Read Button state
  bool new_status = (HAL_GPIO_ReadPin(button.button_port, button.button_pin) == button.pin_state);
    
  // No sense do something if button status already set
  if(button.btn_state != new_status)
  {
    if((button.btn_state_tmp == new_status) && (button.btn_state_cnt == BUTTON_READ_DELAY))
    {
      // If temporary button state true and delay done - update state
      button.btn_state = new_status;
    }
    else if(button.btn_state_tmp == new_status)
    {
      // If temporary button state true and delay not done - increase counter
      button.btn_state_cnt++;
    }
    else
    {
      // If temporary button state true and delay not done - increase counter
      button.btn_state_tmp = new_status;
      button.btn_state_cnt = 0;
    }
  }
}

// *****************************************************************************
// ***   Process Encoders Input function   *************************************
// *****************************************************************************
void InputDrv::ProcessEncodersInput(void)
{
  // Cycle for process devices
  for(uint32_t i = 0U; i < EXT_MAX; i++)
  {
    // Check device type
    if(devices[i] == EXT_DEV_ENC)
    {
      // Process encoder device
      ProcessEncoderInput(encoders[i].enc);
    }
  }
}

// *****************************************************************************
// ***   Process Encoder Input function   **************************************
// *****************************************************************************
void InputDrv::ProcessEncoderInput(EncoderProfile& encoder)
{
  // Read Button state
  uint8_t en_new_status = (HAL_GPIO_ReadPin(encoder.enc_clk_port, encoder.enc_clk_pin) << 1) |
                           HAL_GPIO_ReadPin(encoder.enc_data_port, encoder.enc_data_pin);
    
  // States must be different
  if(en_new_status != encoder.enc_state)
  {
    // We check only rise front on CLK pulse
    if( ((en_new_status >> 1) == 1) && ((encoder.enc_state >> 1) == 0) )
    {
      // State of DATA input must be same during CLK pulse rise 
      if( (en_new_status & 1) == (encoder.enc_state & 1) )
      {
        // If DATA 1 - rotate in one side, if 0 - in another
        if((en_new_status & 1) == 0)
        {
          // Increment encoder counter
          encoder.enc_cnt++;
        }
        else
        {
          // Decrement encoder counter
          encoder.enc_cnt--;
        }
      }
    }
    // Save new encoder state
    encoder.enc_state = en_new_status;
  }
}

// *****************************************************************************
// ***   Process Joystick Input function   *************************************
// *****************************************************************************
void InputDrv::ProcessJoystickInput(JoystickProfile& joystick, PortType port)
{
  // Get value for X channel, add it to previous value
  joystick.x_ch_val += HAL_ADCEx_InjectedGetValue(hadc, ((uint32_t)port << 1U) + 1U);
  joystick.x_ch_val /= 2; // Find average of two values
  // Get value for X channel, add it to previous value
  joystick.y_ch_val += HAL_ADCEx_InjectedGetValue(hadc, ((uint32_t)port << 1U) + 2U);
  joystick.y_ch_val /= 2; // Find average of two values
}

// *****************************************************************************
// ***   Emulate buttons using joystick function   *****************************
// *****************************************************************************
void InputDrv::EmulateButtonsByJoystick(PortType port)
{

  // Center values
  int32_t x_val;
  int32_t y_val;

  // If X channel is inverted
  if(joysticks[port].joy.x_inverted == true)
  {
    // Return inverted X state
    x_val = ADC_MAX_VAL - joysticks[port].joy.x_ch_val - ADC_MAX_VAL/2;
  }
  else
  {
    // Return inverted X state
    x_val = joysticks[port].joy.x_ch_val - ADC_MAX_VAL/2;
  }
  // If Y channel is inverted
  if(joysticks[port].joy.y_inverted == true)
  {
    // Return inverted Y state
    y_val = ADC_MAX_VAL - joysticks[port].joy.y_ch_val - ADC_MAX_VAL/2;
  }
  else
  {
    // Return Y state
    y_val = joysticks[port].joy.y_ch_val - ADC_MAX_VAL/2;
  }

  // Button left
  if(x_val < -JOY_THRESHOLD)
  {
    buttons[port].button[BTN_LEFT].btn_state = true;
  }
  else
  {
    buttons[port].button[BTN_LEFT].btn_state = false;
  }
  // Button right
  if(x_val > JOY_THRESHOLD)
  {
    buttons[port].button[BTN_RIGHT].btn_state = true;
  }
  else
  {
    buttons[port].button[BTN_RIGHT].btn_state = false;
  }
  // Button up
  if(y_val < -JOY_THRESHOLD)
  {
    buttons[port].button[BTN_UP].btn_state = true;
  }
  else
  {
    buttons[port].button[BTN_UP].btn_state = false;
  }
  // Button down
  if(y_val > JOY_THRESHOLD)
  {
    buttons[port].button[BTN_DOWN].btn_state = true;
  }
  else
  {
    buttons[port].button[BTN_DOWN].btn_state = false;
  }
}

// *****************************************************************************
// ***   Emulate encoders using buttons function   *****************************
// *****************************************************************************
void InputDrv::EmulateEncodersByButtons(PortType port)
{
  // Buttons previous states
  static bool btn_left[EXT_MAX];
  static bool btn_right[EXT_MAX];
  // If left button changed
  if(buttons[port].button[BTN_LEFT].btn_state != btn_left[port])
  {
    // Save button state
    btn_left[port] = buttons[port].button[BTN_LEFT].btn_state;
    // If button pressed
    if(btn_left[port])
    {
      // Decrease encoder counter
      encoders[port].enc.enc_cnt--;
    }
  }
  // If right button changed
  if(buttons[port].button[BTN_RIGHT].btn_state != btn_right[port])
  {
    // Save button state
    btn_right[port] = buttons[port].button[BTN_RIGHT].btn_state;
    // If button pressed
    if(btn_right[port])
    {
      // Decrease encoder counter
      encoders[port].enc.enc_cnt++;
    }
  }
  // Copy state of down button to encoder button
  encoders[port].btn[ENC_BTN_ENT] = buttons[port].button[BTN_DOWN];
  encoders[port].btn[ENC_BTN_BACK]= buttons[port].button[BTN_UP];
}

// *****************************************************************************
// ***   Get device type   *****************************************************
// *****************************************************************************
InputDrv::ExtDeviceType InputDrv::GetDeviceType(PortType port)
{
  // Return current state of button
  return devices[port];
}

// *****************************************************************************
// ***   Get button state   ****************************************************
// *****************************************************************************
bool InputDrv::GetButtonState(PortType port, ButtonType button)
{
  // Return current state of button
  return buttons[port].button[button].btn_state;
}

// *****************************************************************************
// ***   Get button state   ****************************************************
// *****************************************************************************
bool InputDrv::GetButtonState(PortType port, ButtonType button, bool& btn_state)
{
  bool ret = false;
  // If button state changed
  if(buttons[port].button[button].btn_state != btn_state)
  {
    // Store new state
    btn_state = buttons[port].button[button].btn_state;
    ret = true;
  }
  // Return result
  return ret;
}

// *****************************************************************************
// ***   Get encoder counts from last call - CAN BE CALLED FROM ONE TASK   *****
// *****************************************************************************
int32_t InputDrv::GetEncoderState(PortType port)
{
  return GetEncoderState(port, last_enc_value[port]);
}

// *****************************************************************************
// ***   Get encoder counts from last call   ***********************************
// *****************************************************************************
int32_t InputDrv::GetEncoderState(PortType port, int32_t& last_enc_val)
{
  // Get current state - atomic operation, prevent multitasking problems
  int32_t enc_val = encoders[port].enc.enc_cnt;
  // Calculate return value
  int32_t retval = enc_val - last_enc_val;
  // Save current count to user provided variable
  last_enc_val = enc_val;
  // return result
  return retval;
}

// *****************************************************************************
// ***   Get encoder button state   ********************************************
// *****************************************************************************
bool InputDrv::GetEncoderButtonCurrentState(PortType port, EncButtonType button)
{
  // Return current state of button
  return encoders[port].btn[button].btn_state;
}

// *****************************************************************************
// ***   Get encoder button state   ********************************************
// *****************************************************************************
bool InputDrv::GetEncoderButtonState(PortType port, EncButtonType button)
{
  return GetEncoderButtonState(port, button, enc_btn_value[port][button]);
}

// *****************************************************************************
// ***   Get encoder button state   ********************************************
// *****************************************************************************
bool InputDrv::GetEncoderButtonState(PortType port, EncButtonType button, bool& btn_state)
{
  bool ret = false;
  // If button state changed
  if(encoders[port].btn[button].btn_state != btn_state)
  {
    // store new state
    btn_state = encoders[port].btn[button].btn_state;
    ret = true;
  }
  // Return result
  return ret;
}

// *****************************************************************************
// ***   Get joystick axis state   *********************************************
// *****************************************************************************
void InputDrv::GetJoystickState(PortType port, int32_t& x, int32_t& y)
{
  // If X channel is inverted
  if(joysticks[port].joy.x_inverted == true)
  {
    // Return inverted X state
    x = ADC_MAX_VAL - joysticks[port].joy.x_ch_val;
  }
  else
  {
    // Return inverted X state
    x = joysticks[port].joy.x_ch_val;
  }
  // If Y channel is inverted
  if(joysticks[port].joy.y_inverted == true)
  {
    // Return inverted Y state
    y = ADC_MAX_VAL - joysticks[port].joy.y_ch_val;
  }
  else
  {
    // Return Y state
    y = joysticks[port].joy.y_ch_val;
  }

  // Calculate X
  if(x > joysticks[port].joy.bx)
  {
    x = ((x - joysticks[port].joy.bx) * joysticks[port].joy.kxmax) / COEF;
  }
  else
  {
    x = ((x - joysticks[port].joy.bx) * joysticks[port].joy.kxmin) / COEF;
  }
  // Calculate Y
  if(y > joysticks[port].joy.by)
  {
    y = ((y - joysticks[port].joy.by) * joysticks[port].joy.kymax) / COEF;
  }
  else
  {
    y = ((y - joysticks[port].joy.by) * joysticks[port].joy.kymin) / COEF;
  }
}

// *****************************************************************************
// ***   SetJoystickCalibrationConsts   ****************************************
// *****************************************************************************
// * Set calibration constants. Must be call for calibration joystick.
void InputDrv::SetJoystickCalibrationConsts(PortType port, int32_t x_mid,
                                            int32_t x_kmin, int32_t x_kmax,
                                            int32_t y_mid, int32_t y_kmin,
                                            int32_t y_kmax)
{
  // X axis calibration
  joysticks[port].joy.bx = x_mid;
  joysticks[port].joy.kxmin = x_kmin;
  joysticks[port].joy.kxmax = x_kmax;
  // Y axis calibration
  joysticks[port].joy.by = y_mid;
  joysticks[port].joy.kymin = y_kmin;
  joysticks[port].joy.kymax = y_kmax;
}

// *****************************************************************************
// ***   Get joystick button state   *******************************************
// *****************************************************************************
bool InputDrv::GetJoystickButtonState(PortType port)
{
  // Return current state of button
  return joysticks[port].btn.btn_state;
}

// *****************************************************************************
// ***   Get joystick button state   *******************************************
// *****************************************************************************
bool InputDrv::GetJoystickButtonState(PortType port, bool& btn_state)
{
  bool ret = false;
  // If button state changed
  if(joysticks[port].btn.btn_state != btn_state)
  {
    // store new state
    btn_state = joysticks[port].btn.btn_state;
    ret = true;
  }
  // Return result
  return ret;
}

// *****************************************************************************
// ***   Configure inputs devices types   **************************************
// *****************************************************************************
InputDrv::ExtDeviceType InputDrv::DetectDeviceType(PortType port)
{
  // Return variable
  ExtDeviceType ret = EXT_DEV_NONE;
  // Variable to store X axis value
  int32_t x_val;
  // Variable to store Y axis value
  int32_t y_val;

  if(hadc != nullptr)
  {
    // Config ADC for measure
    if(port == EXT_LEFT)  ConfigADC(EXT_DEV_JOY,  EXT_DEV_NONE);
    if(port == EXT_RIGHT) ConfigADC(EXT_DEV_NONE, EXT_DEV_JOY);
    // Start measurement
    HAL_ADCEx_InjectedStart(hadc);
    // Wait until End of Conversion flag is raised
    while(HAL_IS_BIT_CLR(hadc->Instance->SR, ADC_FLAG_JEOC));
    x_val = HAL_ADCEx_InjectedGetValue(hadc, (port << 1) + 1);
    y_val = HAL_ADCEx_InjectedGetValue(hadc, (port << 1) + 2);

    // Center values
    x_val -= ADC_MAX_VAL/2;
    y_val -= ADC_MAX_VAL/2;
    // If at least one value near center
    if(   ((x_val > -JOY_THRESHOLD) && (x_val < JOY_THRESHOLD))
       || ((y_val > -JOY_THRESHOLD) && (y_val < JOY_THRESHOLD)))
    {
      // Joystick connected
      ret = EXT_DEV_JOY;
    }
    else
    {
      // Stop ADC before switch to digital inputs
      HAL_ADCEx_InjectedStop(hadc);
      // Config input IO to digital
      ConfigInputIO(true, port);
    }
  }
  else
  {
    // Config input IO to digital
    ConfigInputIO(true, port);
  }

  // If Joystick not detected
  if(ret == EXT_DEV_NONE)
  {
    // Configure GPIO pins for detect devices
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Pin = encoders[port].btn[ENC_BTN_BACK].button_pin;
    HAL_GPIO_Init(encoders[port].btn[ENC_BTN_BACK].button_port, &GPIO_InitStruct);

    // If pin state low - encoder
    if(HAL_GPIO_ReadPin(encoders[port].btn[ENC_BTN_BACK].button_port,
                        encoders[port].btn[ENC_BTN_BACK].button_pin) == GPIO_PIN_RESET)
    {
      ret = EXT_DEV_ENC;
    }
    else
    {
      ret = EXT_DEV_BTN;
    }

    // Restore GPIO pins configuration
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = buttons[port].button[BTN_LEFT].button_pin;
    HAL_GPIO_Init(buttons[port].button[BTN_LEFT].button_port, &GPIO_InitStruct);
  }
  return ret;
}

// *****************************************************************************
// ***   Config ADC   **********************************************************
// *****************************************************************************
void InputDrv::ConfigADC(ExtDeviceType dev_left, ExtDeviceType dev_right)
{
  // We can configure ADC only if ADC handle is not nullptr and at least one
  // joystick present
  if(   (hadc != nullptr)
     && ((dev_left == EXT_DEV_JOY) || (dev_right == EXT_DEV_JOY)))
  {
    // Update ADC settings - should be Scan mode
#if defined(STM32F1)
    hadc->Init.ScanConvMode = ADC_SCAN_ENABLE;
#elif defined(STM32F4)
    hadc->Init.ScanConvMode = ENABLE;
#endif
    // Init ADC
    if (HAL_ADC_Init(hadc) != HAL_OK)
    {
      Error_Handler();
    }

    // Structure for init Injected Channel
    ADC_InjectionConfTypeDef sConfigInjected;
    // Configure Injected Channel
    sConfigInjected.InjectedNbrOfConversion = EXT_MAX*2;
#if defined(STM32F1)
    sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_239CYCLES_5;
#elif defined(STM32F4)
    sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_480CYCLES;
#endif
    sConfigInjected.ExternalTrigInjecConv = ADC_INJECTED_SOFTWARE_START;
    sConfigInjected.AutoInjectedConv = DISABLE;
    sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
    sConfigInjected.InjectedOffset = 0;
    sConfigInjected.InjectedRank = 1;

    // Channels
    uint32_t injected_channels[EXT_MAX*2] = {0,0,0,0};

    // If both devices - joysticks
    if((dev_left == EXT_DEV_JOY) && (dev_right == EXT_DEV_JOY))
    {
      // Left joystick
      injected_channels[0] = joysticks[EXT_LEFT].joy.x_channel;
      injected_channels[1] = joysticks[EXT_LEFT].joy.y_channel;
      // Right joystick
      injected_channels[2] = joysticks[EXT_RIGHT].joy.x_channel;
      injected_channels[3] = joysticks[EXT_RIGHT].joy.y_channel;
    }
    // If only left joystick present
    else if(dev_left == EXT_DEV_JOY)
    {
      // Left joystick for all channels
      injected_channels[0] = joysticks[EXT_LEFT].joy.x_channel;
      injected_channels[1] = joysticks[EXT_LEFT].joy.y_channel;
      injected_channels[2] = joysticks[EXT_LEFT].joy.x_channel;
      injected_channels[3] = joysticks[EXT_LEFT].joy.y_channel;
    }
    // If only right joystick present
    else if(dev_right == EXT_DEV_JOY)
    {
      // Right joystick for all channels
      injected_channels[0] = joysticks[EXT_RIGHT].joy.x_channel;
      injected_channels[1] = joysticks[EXT_RIGHT].joy.y_channel;
      injected_channels[2] = joysticks[EXT_RIGHT].joy.x_channel;
      injected_channels[3] = joysticks[EXT_RIGHT].joy.y_channel;
    }
    else
    {
      // FIX ME: CATCH ERROR HERE !!!
    }

    // Cycle for init Injected channels
    for(uint32_t i = 0U; i < NumberOf(injected_channels); i++)
    {
      // Configure Injected Channel - X axis
      sConfigInjected.InjectedChannel = injected_channels[i];
      if (HAL_ADCEx_InjectedConfigChannel(hadc, &sConfigInjected) != HAL_OK)
      {
        Error_Handler();
      }
      // Increase rank counter
      sConfigInjected.InjectedRank++;
    }
  }
}

// *****************************************************************************
// ***   Configure inputs for read digital/analog data   ***********************
// *****************************************************************************
void InputDrv::ConfigInputIO(bool is_digital, PortType port)
{
  // Structure for set IO parameters
  GPIO_InitTypeDef GPIO_InitStruct;
  // Set no pull - no effect if analog mode selected
  GPIO_InitStruct.Pull = GPIO_NOPULL;

  // Check requested mode
  if(is_digital == true)
  {
    // Set input mode
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  }
  else
  {
    // Set analog mode
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  }

  // Configure GPIO pins : EXT_L1_Pin
  GPIO_InitStruct.Pin = joysticks[port].joy.x_pin;
  HAL_GPIO_Init(joysticks[port].joy.x_port, &GPIO_InitStruct);
  // Configure GPIO pins : EXT_L2_Pin
  GPIO_InitStruct.Pin = joysticks[port].joy.y_pin;
  HAL_GPIO_Init(joysticks[port].joy.y_port, &GPIO_InitStruct);
}

#endif
