//******************************************************************************
//  @file StHalUart.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: STM32 HAL UART driver, implementation
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
#include "StHalUart.h"

#include <cstring> // for memset()

// *****************************************************************************
// ***   This driver can be compiled only if UART configured in CubeMX   *******
// *****************************************************************************
#if defined(HAL_USART_MODULE_ENABLED) || defined(HAL_UART_MODULE_ENABLED)

// *****************************************************************************
// ***   Public: Init   ********************************************************
// *****************************************************************************
Result StHalUart::Init()
{
  return StartRx();
}

// *****************************************************************************
// ***   Public: DeInit   ******************************************************
// *****************************************************************************
Result StHalUart::DeInit()
{
  Result result = Result::RESULT_OK;

  if(HAL_UART_Abort(&huart) != HAL_OK)
  {
    result = Result::ERR_UART_GENERAL;
  }

  return result;
}

// *****************************************************************************
// ***   Public: Read   ********************************************************
// *****************************************************************************
Result StHalUart::Read(uint8_t& value)
{
  Result result = Result::RESULT_OK;

  // Calculate index in circular buffer
  uint16_t index = RX_BUF_SIZE - ndtr;

  // TODO: is it good enough?
  // If an error happened
  if(__HAL_UART_GET_FLAG(&huart, UART_FLAG_ORE) || __HAL_UART_GET_FLAG(&huart, UART_FLAG_FE))
  {
    // Restart RX
    StartRx();
  }

  // Check data
  if(ndtr != __HAL_DMA_GET_COUNTER(huart.hdmarx))
  {
    // Store value
    value = rx_buf[index];
    // Decrease counter
    ndtr--;
    // Check reset
    if(ndtr == 0U)
    {
      ndtr = RX_BUF_SIZE;
    }
  }
  else
  {
    // No data
    result = Result::ERR_UART_EMPTY;
  }

  return result;
}

// *****************************************************************************
// ***   Public: Read   ********************************************************
// *****************************************************************************
Result StHalUart::Read(uint8_t* rx_buf_ptr, uint32_t& size)
{
  Result result = Result::RESULT_OK;

  uint16_t rx_idx = 0U;
  // Read bytes in cycle
  while(rx_idx < size)
  {
    // Temporary variable for received byte
    uint8_t rx_byte;
    // Read byte - this function overwrite byte in case of overrun. So,
    // we use additional variable here. May be it isn't so necessary.
    result = Read(rx_byte);
    // If read successful
    if(result.IsGood())
    {
      // Save received byte
      rx_buf_ptr[rx_idx] = rx_byte;
      // Increment index
      rx_idx++;
    }
    else
    {
      // Exit from the cycle
      break;
    }
  }
  // Save read bytes count
  size = rx_idx;
  // If we received any number of bytes - override "empty" error
  if((rx_idx != 0U) && (result == Result::ERR_UART_EMPTY))
  {
    result = Result::RESULT_OK;
  }

  return result;
}

// *****************************************************************************
// ***   Public: Write   *******************************************************
// *****************************************************************************
Result StHalUart::Write(uint8_t* tx_buf_ptr, uint32_t size)
{
  Result result = Result::ERR_NULL_PTR;

  // Check input parameters
  if(tx_buf_ptr != nullptr)
  {
    // Transfer data
    HAL_StatusTypeDef hal_result = HAL_UART_Transmit_DMA(&huart, tx_buf_ptr, size);

    // Convert operation result to Result
    if(hal_result == HAL_BUSY)
    {
      // Busy error
      result = Result::ERR_UART_BUSY;
    }
    else if(hal_result != HAL_OK)
    {
      // All other errors
      result = Result::ERR_UART_TRANSMIT;
    }
    else
    {
      result = Result::RESULT_OK;
    }
  }

  return result;
}

// *****************************************************************************
// ***   Public: IsTxComplete   ************************************************
// *****************************************************************************
bool StHalUart::IsTxComplete(void)
{
  // Check gState
  return (huart.gState == HAL_UART_STATE_READY);
}

// *****************************************************************************
// ***   Private: GetRxSize   **************************************************
// *****************************************************************************
Result StHalUart::GetRxSize(uint16_t& rx_cnt)
{
  // Get DMA counter from register
  uint16_t new_ndtr = __HAL_DMA_GET_COUNTER(huart.hdmarx);
  // Calculate number of received bytes
  if(ndtr >= new_ndtr)
  {
    rx_cnt = ndtr - new_ndtr;
  }
  else
  {
    rx_cnt = huart.RxXferSize - (new_ndtr - ndtr);
  }

  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Private: StartRx   ****************************************************
// *****************************************************************************
Result StHalUart::StartRx()
{
  Result result = Result::RESULT_OK;

  // Clear array
  memset(rx_buf, 0xFFU, sizeof(rx_buf));

  // Start the RX DMA - this is circular, so it will never stop
  if(HAL_UART_Receive_DMA(&huart, rx_buf, RX_BUF_SIZE) == HAL_OK)
  {
    // This is the first access to this object and ndtr must be initialized
    ndtr = huart.RxXferSize;
  }
  else
  {
    result = Result::ERR_UART_RECEIVE;
  }

  return Result::RESULT_OK;
}

#endif
