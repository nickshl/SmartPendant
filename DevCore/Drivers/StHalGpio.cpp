//******************************************************************************
//  @file StHalGpio.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: STM32 HAL GPIO driver, implementation
//
//  @copyright Copyright (c) 2018, Devtronic & Nicolai Shlapunov
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
#include "StHalGpio.h"

// *****************************************************************************
// ***   This driver can be compiled only if GPIO configured in CubeMX   *******
// *****************************************************************************
#ifdef HAL_GPIO_MODULE_ENABLED

// *****************************************************************************
// ***   Public: Read   ********************************************************
// *****************************************************************************
StHalGpio::Polarity StHalGpio::Read()
{
  // Get pin state
  GPIO_PinState pin = HAL_GPIO_ReadPin(gpio_port, pin_mask_bit);
  // Return result
  return (pin == GPIO_PIN_SET ? HIGH : LOW);
}

// *****************************************************************************
// ***   Public: Write   *******************************************************
// *****************************************************************************
void StHalGpio::Write(StHalGpio::Polarity polarity)
{
  // Convert polarity to pin state
  GPIO_PinState pin_state = (polarity == HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET;
  // Write to hardware
  HAL_GPIO_WritePin(gpio_port, pin_mask_bit, pin_state);
}

#endif
