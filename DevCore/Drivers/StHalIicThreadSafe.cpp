//******************************************************************************
//  @file StHalIicThreadSafe.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: STM32 HAL I2C thread safe driver, implementation
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
#include "StHalIicThreadSafe.h"

// *****************************************************************************
// ***   This driver can be compiled only if I2C configured in CubeMX   ********
// *****************************************************************************
#ifdef HAL_I2C_MODULE_ENABLED

// *****************************************************************************
// ***   Public: Enable   ******************************************************
// *****************************************************************************
Result StHalIicThreadSafe::Enable()
{
  // Set PE bit
  __HAL_I2C_ENABLE(&hi2c);
  // No errors to return
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Public: Disable   *****************************************************
// *****************************************************************************
Result StHalIicThreadSafe::Disable()
{
  // Since this class can be used by multiple tasks, we can't disable I2C
  // by request from one of them.
  return Result::ERR_NOT_IMPLEMENTED;
}

// *****************************************************************************
// ***   Public: Reset   *******************************************************
// *****************************************************************************
Result StHalIicThreadSafe::Reset()
{
  // Lock Mutex
  mutex.Lock();
  // Clear PE bit
  CLEAR_BIT(hi2c.Instance->CR1, I2C_CR1_PE);
  // PE must be kept low during at least 3 APB clock cycles in order to
  // perform the software reset. Wait until it actually cleared.
  while(READ_BIT(hi2c.Instance->CR1, I2C_CR1_PE));
  // Set Software reset bit
  SET_BIT(hi2c.Instance->CR1, I2C_CR1_SWRST);
  // Clear Software reset bit
  CLEAR_BIT(hi2c.Instance->CR1, I2C_CR1_SWRST);

  // TODO: we need to make some clock pulses on the SCL line here. Sometimes
  // In some cases when slave missed clock pulse by some reason it may stuck in
  // acknowledge state and pull SDA line low. MCU will see that and consider
  // line busy. It can be fixed by producing another pulse on SCL line which
  // should force slave to release SDA line.
  // However, it not easy(universal) to do since hi2c does not contains GPIO
  // info for I2C pins.

  // Reinit H2C(PE bit will be set by this function)
  HAL_I2C_Init(&hi2c);
  // Release Mutex
  mutex.Release();
  // No errors to return
  return Result::RESULT_OK;
}

// *************************************************************************
// ***   Public: IsDeviceReady   *******************************************
// *************************************************************************
Result StHalIicThreadSafe::IsDeviceReady(uint16_t addr, uint8_t retries)
{
  Result result;
  // Shift address one bit left - HAL blow away LSB, not MSB.
  addr <<= 1U;
  // Lock Mutex
  mutex.Lock();
  // Check device status
  HAL_StatusTypeDef hal_result = HAL_I2C_IsDeviceReady(&hi2c, addr, retries, i2c_tx_timeout_ms);
  // Release Mutex
  mutex.Release();
  // Convert operation result to Result
  result = ConvertResult(hal_result);
  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: Transfer   ****************************************************
// *****************************************************************************
Result StHalIicThreadSafe::Transfer(uint16_t addr, uint8_t* tx_buf_ptr, uint32_t tx_size, uint8_t* rx_buf_ptr, uint32_t rx_size)
{
  Result result = Result::ERR_NULL_PTR;

  // Lock Mutex
  mutex.Lock();

  // Special hack for use HAL_I2C_Mem_Read() function for send Repeated Start if
  // tx_size is 1 or 2 bytes.
  if((tx_buf_ptr != nullptr) && (rx_buf_ptr != nullptr) && ((tx_size == 1U) || (tx_size == 2U)))
  {
    // Variable for store result from the HAL
    HAL_StatusTypeDef hal_result = HAL_OK;

    // Shift address one bit left - HAL blow away LSB, not MSB.
    addr <<= 1U;

    // Transmit data
    hal_result = HAL_I2C_Mem_Read(&hi2c, addr, tx_buf_ptr[0], tx_size, rx_buf_ptr, rx_size, i2c_tx_timeout_ms);

    // Convert operation result to Result
    result = ConvertResult(hal_result);
  }
  else
  {
    if(tx_buf_ptr != nullptr)
    {
      // Transmit data
      result = Write(addr, tx_buf_ptr, tx_size);
    }

    if((rx_buf_ptr != nullptr) && result.IsGood())
    {
      // Clear RX buffer
      for(uint32_t i = 0; i < rx_size; i++)
      {
        rx_buf_ptr[i] = 0;
      }
      // Receive data
      result = Read(addr, rx_buf_ptr, rx_size);
    }
  }

  // Release Mutex
  mutex.Release();

  return result;
}

// *****************************************************************************
// ***   Public: Write   *******************************************************
// *****************************************************************************
Result StHalIicThreadSafe::Write(uint16_t addr, uint8_t* tx_buf_ptr, uint32_t tx_size)
{
  Result result = Result::ERR_NULL_PTR;

  if(tx_buf_ptr != nullptr)
  {
    // Variable for store result from the HAL
    HAL_StatusTypeDef hal_result = HAL_OK;

    // Shift address one bit left - HAL blow away LSB, not MSB.
    addr <<= 1U;

    // Lock Mutex
    mutex.Lock();
    // Transmit data
    hal_result = HAL_I2C_Master_Transmit(&hi2c, addr, tx_buf_ptr, tx_size, i2c_tx_timeout_ms);
    // Release Mutex
    mutex.Release();

    // Convert operation result to Result
    result = ConvertResult(hal_result);
  }

  return result;
}

// *****************************************************************************
// ***   Public: Read   ********************************************************
// *****************************************************************************
Result StHalIicThreadSafe::Read(uint16_t addr, uint8_t* rx_buf_ptr, uint32_t rx_size)
{
  Result result = Result::ERR_NULL_PTR;

  if(rx_buf_ptr != nullptr)
  {
    // Variable for store result from the HAL
    HAL_StatusTypeDef hal_result = HAL_OK;

    // Shift address one bit left - HAL blow away LSB, not MSB.
    addr <<= 1U;

    // Lock Mutex
    mutex.Lock();
    // Transmit data
    hal_result = HAL_I2C_Master_Receive(&hi2c, addr, rx_buf_ptr, rx_size, i2c_tx_timeout_ms);
    // Release Mutex
    mutex.Release();

    // Convert operation result to Result
    result = ConvertResult(hal_result);
  }

  return result;
}

// *****************************************************************************
// ***   Public: WriteAsync   **************************************************
// *****************************************************************************
Result StHalIicThreadSafe::WriteAsync(uint16_t addr, uint8_t* tx_buf_ptr, uint32_t tx_size)
{
  Result result = Result::ERR_NOT_IMPLEMENTED;

  // Check DMA handler - if it is nullptr this function not implemented in hardware
  if(hi2c.hdmatx != nullptr)
  {
    // Shift address one bit left - HAL blow away LSB, not MSB.
    addr <<= 1U;
    // Lock Mutex
    mutex.Lock();
    // Check if there no ongoing transfer, if there is, we will stuck there
    while(IsBusy()) RtosTick::DelayTicks(1u);
    // Receive data using DMA
    HAL_StatusTypeDef hal_result = HAL_I2C_Master_Transmit_DMA(&hi2c, addr, tx_buf_ptr, tx_size);
    // Release Mutex
    mutex.Release();
    // Convert operation result to Result
    result = ConvertResult(hal_result);
  }

  return result;
}

// *****************************************************************************
// ***   Public: ReadAsync   ***************************************************
// *****************************************************************************
Result StHalIicThreadSafe::ReadAsync(uint16_t addr, uint8_t* rx_buf_ptr, uint32_t rx_size)
{
  Result result = Result::ERR_NOT_IMPLEMENTED;

  // Check DMA handler - if it is nullptr this function not implemented in hardware
  if(hi2c.hdmarx != nullptr)
  {
    // Shift address one bit left - HAL blow away LSB, not MSB.
    addr <<= 1U;
    // Lock Mutex
    mutex.Lock();
    // Check if there no ongoing transfer, if there is, we will stuck there
    while(IsBusy()) RtosTick::DelayTicks(1u);
    // Receive data using DMA
    HAL_StatusTypeDef hal_result = HAL_I2C_Master_Receive_DMA(&hi2c, addr, rx_buf_ptr, rx_size);
    // Release Mutex
    mutex.Release();
    // Convert operation result to Result
    result = ConvertResult(hal_result);
  }

  return result;
}

// *****************************************************************************
// ***   Public: Transfer   ****************************************************
// *****************************************************************************
bool StHalIicThreadSafe::IsBusy(void)
{
   return (hi2c.State != HAL_I2C_STATE_READY);
}

// *****************************************************************************
// ***   Private: ConvertResult   **********************************************
// *****************************************************************************
Result StHalIicThreadSafe::ConvertResult(HAL_StatusTypeDef hal_result)
{
  Result result = Result::RESULT_OK;

  // Convert operation result to Result
  switch(hal_result)
  {
    case HAL_OK:
      result = Result::RESULT_OK;
      break;

    case HAL_ERROR:
      result = Result::ERR_I2C_GENERAL;
      break;

    case HAL_BUSY:
      result = Result::ERR_I2C_BUSY;
      break;

    case HAL_TIMEOUT:
      result = Result::ERR_I2C_TIMEOUT;
      break;

    default:
      result = Result::ERR_SPI_UNKNOWN;
      break;
  }

  // Return result
  return result;
}

#endif
