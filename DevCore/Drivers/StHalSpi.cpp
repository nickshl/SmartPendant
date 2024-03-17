//******************************************************************************
//  @file StHalIic.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: STM32 HAL SPI driver, implementation
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
#include "StHalSpi.h"

// *****************************************************************************
// ***   This driver can be compiled only if SPI configured in CubeMX   ********
// *****************************************************************************
#ifdef HAL_SPI_MODULE_ENABLED

// *****************************************************************************
// ***   Public: Transfer   ****************************************************
// *****************************************************************************
Result StHalSpi::Transfer(uint8_t* tx_buf_ptr, uint8_t* rx_buf_ptr, uint32_t size)
{
  Result result = Result::ERR_NULL_PTR;

  // Check for null pointer
  if((tx_buf_ptr != nullptr) && (rx_buf_ptr != nullptr))
  {
    // Read data from SPI port
    HAL_StatusTypeDef hal_result = HAL_SPI_TransmitReceive(&hspi, tx_buf_ptr, rx_buf_ptr, size, spi_tx_timeout_ms);
    // Convert operation result to Result
    result = ConvertResult(hal_result);
  }

  return result;
}

// *****************************************************************************
// ***   Public: Write   *******************************************************
// *****************************************************************************
Result StHalSpi::Write(uint8_t* tx_buf_ptr, uint32_t size)
{
  Result result = Result::ERR_NULL_PTR;

  // Check for null pointer
  if (tx_buf_ptr != nullptr)
  {
    // Read data from SPI port
    HAL_StatusTypeDef hal_result = HAL_SPI_Transmit(&hspi, tx_buf_ptr, size, spi_tx_timeout_ms);
    // Convert operation result to Result
    result = ConvertResult(hal_result);
  }

  return result;
}

// *****************************************************************************
// ***   Public: Read   ********************************************************
// *****************************************************************************
Result StHalSpi::Read(uint8_t* rx_buf_ptr, uint32_t size)
{
  Result result = Result::RESULT_OK;

  // Check for null pointer
  if (rx_buf_ptr != nullptr)
  {
    // Read data from the SPI port
    HAL_StatusTypeDef hal_result = HAL_SPI_Receive(&hspi, rx_buf_ptr, size, spi_rx_timeout_ms);
    // Convert operation result to Result
    result = ConvertResult(hal_result);
  }

  return result;
}

// *****************************************************************************
// ***   Public: TransferAsync   ***********************************************
// *****************************************************************************
Result StHalSpi::TransferAsync(uint8_t* tx_buf_ptr, uint8_t* rx_buf_ptr, uint32_t size)
{
  Result result = Result::RESULT_OK;

  // Check for null pointer
  if((tx_buf_ptr != nullptr) && (rx_buf_ptr != nullptr))
  {
    // Read data from SPI port
    HAL_StatusTypeDef hal_result = HAL_SPI_TransmitReceive_DMA(&hspi, tx_buf_ptr, rx_buf_ptr, size);
    // Convert operation result to Result
    result = ConvertResult(hal_result);
  }

  return result;
}

// *****************************************************************************
// ***   Public: WriteAsync   **************************************************
// *****************************************************************************
Result StHalSpi::WriteAsync(uint8_t* tx_buf_ptr, uint32_t size)
{
  Result result = Result::RESULT_OK;

  // Check for null pointer
  if(tx_buf_ptr != nullptr)
  {
    // Read data from  SPI port
    HAL_StatusTypeDef hal_result = HAL_SPI_Transmit_DMA(&hspi, tx_buf_ptr, size);
    // Convert operation result to Result
    result = ConvertResult(hal_result);
  }

  return result;
}

// *****************************************************************************
// ***   Public: ReadAsync   ***************************************************
// *****************************************************************************
Result StHalSpi::ReadAsync(uint8_t* rx_buf_ptr, uint32_t size)
{
  Result result = Result::RESULT_OK;

  // Check for null pointer
  if(rx_buf_ptr != nullptr)
  {
    // Read data from SPI port
    HAL_StatusTypeDef hal_result = HAL_SPI_Receive_DMA(&hspi, rx_buf_ptr, size);
    // Convert operation result to Result
    result = ConvertResult(hal_result);
  }

  return result;
}

// *****************************************************************************
// ***   Public: Check SPI transfer status   ***********************************
// *****************************************************************************
bool StHalSpi::IsTransferComplete(void)
{
  return(hspi.State != HAL_SPI_STATE_BUSY_TX);
}

// *****************************************************************************
// ***   Public: Abort   *******************************************************
// *****************************************************************************
Result StHalSpi::Abort(void)
{
  Result result = Result::RESULT_OK;

  // Abort SPI transmissions data from SPI port
  HAL_StatusTypeDef hal_result = HAL_SPI_Abort(&hspi); // TODO: what difference from HAL_SPI_Abort_IT(&hspi) ???
  // Convert operation result to Result
  result = ConvertResult(hal_result);

  return result;
}

// *************************************************************************
// ***   Public: SetSpeed   ************************************************
// *************************************************************************
Result StHalSpi::SetSpeed(uint32_t clock_rate)
{
  Result result = Result::RESULT_OK;

  // Get SPI bus clock
  uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();
  // Find divider
  uint32_t divider = pclk2 / clock_rate;
  // Variable for baud prescaler
  uint32_t baud_prescaler = SPI_BAUDRATEPRESCALER_2;

  // Set prescaler for SPI
  if(divider <= 2U)
  {
    baud_prescaler = SPI_BAUDRATEPRESCALER_2;
    clock_rate = pclk2 / 2U;
  }
  else if(divider <= 4U)
  {
    baud_prescaler = SPI_BAUDRATEPRESCALER_4;
    clock_rate = pclk2 / 4U;
  }
  else if(divider <= 8U)
  {
    baud_prescaler = SPI_BAUDRATEPRESCALER_8;
    clock_rate = pclk2 / 8U;
  }
  else if(divider <= 16U)
  {
    baud_prescaler = SPI_BAUDRATEPRESCALER_16;
    clock_rate = pclk2 / 16U;
  }
  else if(divider <= 32U)
  {
    baud_prescaler = SPI_BAUDRATEPRESCALER_32;
    clock_rate = pclk2 / 32U;
  }
  else if(divider <= 64U)
  {
    baud_prescaler = SPI_BAUDRATEPRESCALER_64;
    clock_rate = pclk2 / 64U;
  }
  else if(divider <= 128U)
  {
    baud_prescaler = SPI_BAUDRATEPRESCALER_128;
    clock_rate = pclk2 / 128U;
  }
  else if(divider <= 256U)
  {
    baud_prescaler = SPI_BAUDRATEPRESCALER_256;
    clock_rate = pclk2 / 256U;
  }
  else
  {
    // We can't set requested or lower speed
    result = Result::ERR_SPI_GENERAL;
  }

  if(result.IsGood())
  {
    // Disable SPI peripheral
    MODIFY_REG(hspi.Instance->CR1, (uint32_t)(SPI_CR1_SPE_Msk), 0U);
    // Set prescaler
    MODIFY_REG(hspi.Instance->CR1, (uint32_t)SPI_CR1_BR_Msk, baud_prescaler);
    // Enable SPI peripheral
    MODIFY_REG(hspi.Instance->CR1, (uint32_t)(SPI_CR1_SPE_Msk), (uint32_t)(SPI_CR1_SPE));
  }

  return result;
}

// *************************************************************************
// ***   Public: GetSpeed   ************************************************
// *************************************************************************
Result StHalSpi::GetSpeed(uint32_t& clock_rate)
{
  Result result = Result::RESULT_OK;

  // Get SPI bus clock
  uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();
  // Get prescaler
  uint32_t baud_prescaler = READ_REG(hspi.Instance->CR1) & (uint32_t)SPI_CR1_BR_Msk;

  // Set prescaler for SPI
  if(baud_prescaler == SPI_BAUDRATEPRESCALER_2)
  {
    clock_rate = pclk2 / 2U;
  }
  else if(baud_prescaler == SPI_BAUDRATEPRESCALER_4)
  {
    clock_rate = pclk2 / 4U;
  }
  else if(baud_prescaler == SPI_BAUDRATEPRESCALER_8)
  {
    clock_rate = pclk2 / 8U;
  }
  else if(baud_prescaler == SPI_BAUDRATEPRESCALER_16)
  {
    clock_rate = pclk2 / 16U;
  }
  else if(baud_prescaler == SPI_BAUDRATEPRESCALER_32)
  {
    clock_rate = pclk2 / 32U;
  }
  else if(baud_prescaler == SPI_BAUDRATEPRESCALER_64)
  {
    clock_rate = pclk2 / 64U;
  }
  else if(baud_prescaler == SPI_BAUDRATEPRESCALER_128)
  {
    clock_rate = pclk2 / 128U;
  }
  else if(baud_prescaler == SPI_BAUDRATEPRESCALER_256)
  {
    clock_rate = pclk2 / 256U;
  }
  else
  {
    // Clear result clock rate
    clock_rate = 0U;
    // We shouldn't get here
    result = Result::ERR_SPI_GENERAL;
  }

  return result;
}

// *************************************************************************
// ***   Public: SetMode   *************************************************
// *************************************************************************
Result StHalSpi::SetMode(ISpi::Mode mode)
{
  Result result = Result::RESULT_OK;

  // Variables for store CPHA & CPOL
  uint32_t setup = 0U;

  // Find CPHA & CPOL by mode
  if(mode == ISpi::MODE_0)
  {
    setup = 0U;
  }
  else if(mode == ISpi::MODE_1)
  {
    setup = SPI_CR1_CPHA;
  }
  else if(mode == ISpi::MODE_2)
  {
    setup = SPI_CR1_CPOL;
  }
  else if(mode == ISpi::MODE_3)
  {
    setup = SPI_CR1_CPHA | SPI_CR1_CPOL;
  }
  else
  {
    result = Result::ERR_BAD_PARAMETER;
  }

  if(result.IsGood())
  {
    // Disable SPI peripheral
    MODIFY_REG(hspi.Instance->CR1, (uint32_t)(SPI_CR1_SPE_Msk), 0U);
    // Set mode
    MODIFY_REG(hspi.Instance->CR1, (uint32_t)(SPI_CR1_CPHA_Msk | SPI_CR1_CPOL_Msk), setup);
    // Enable SPI peripheral
    MODIFY_REG(hspi.Instance->CR1, (uint32_t)(SPI_CR1_SPE_Msk), (uint32_t)(SPI_CR1_SPE));
  }

  return result;
}

// *************************************************************************
// ***   Public: GetMode   *************************************************
// *************************************************************************
Result StHalSpi::GetMode(ISpi::Mode& mode)
{
  Result result = Result::RESULT_OK;

  // Get current SPI settings
  uint32_t settings = READ_REG(hspi.Instance->CR1);
  // Find CPHA & CPOL
  bool cpha = settings & SPI_CR1_CPHA_Msk;
  bool cpol = settings & SPI_CR1_CPOL_Msk;

  // Find mode by CPHA & CPOL
  if((cpha == false) && (cpol == false))
  {
    mode = ISpi::MODE_0;
  }
  else if((cpha == true) && (cpol == false))
  {
    mode = ISpi::MODE_1;
  }
  else if((cpha == false) && (cpol == true))
  {
    mode = ISpi::MODE_2;
  }
  else if((cpha == true) && (cpol == true))
  {
    mode = ISpi::MODE_3;
  }
  else
  {
    result = Result::ERR_BAD_PARAMETER;
  }

  // Always Ok
  return result;
}

// *****************************************************************************
// ***   Private: GetToDataSizeBytes   *****************************************
// *****************************************************************************
uint32_t StHalSpi::GetToDataSizeBytes()
{
   // Shift by a byte and add one because 0x300 is 4 bits
   return(hspi.Init.DataSize >> 8U) + 1U;
}

// *****************************************************************************
// ***   Private: ConvertResult   **********************************************
// *****************************************************************************
Result StHalSpi::ConvertResult(HAL_StatusTypeDef hal_result)
{
  Result result = Result::RESULT_OK;

  // Convert operation result to Result
  switch(hal_result)
  {
    case HAL_OK:
      result = Result::RESULT_OK;
      break;

    case HAL_ERROR:
      result = Result::ERR_SPI_GENERAL;
      break;

    case HAL_BUSY:
      result = Result::ERR_SPI_BUSY;
      break;

    case HAL_TIMEOUT:
      result = Result::ERR_SPI_TIMEOUT;
      break;

    default:
      result = Result::ERR_SPI_UNKNOWN;
      break;
  }

  // Return result
  return result;
}

#endif
