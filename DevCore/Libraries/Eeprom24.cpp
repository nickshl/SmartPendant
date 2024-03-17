//******************************************************************************
//  @file Eeprom24.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: EEPROM 24C*** driver, implementation
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
#include "Eeprom24.h"

#include "string.h" // for memcpy()

// *****************************************************************************
// ***   Public: Init   ********************************************************
// *****************************************************************************
Result Eeprom24::Init()
{
  Result result = Result::RESULT_OK;
  iic.SetTxTimeout(10U);
  iic.SetRxTimeout(100U);
  // Enable write protection
  if(write_protection != nullptr)
  {
    write_protection->SetHigh();
  }
  return result;
}

// *****************************************************************************
// ***   Public: Read   ********************************************************
// *****************************************************************************
Result Eeprom24::Read(uint16_t addr, uint8_t* rx_buf_ptr, uint16_t size)
{
  Result result = Result::RESULT_OK;

  // Check input parameters
  if(rx_buf_ptr == nullptr)
  {
    // Set error
    result = Result::ERR_NULL_PTR;
  }
  else if(addr + size > size_bytes)
  {
    // Set error
    result = Result::ERR_INVALID_SIZE;
  }
  else
  {
    // Transfer: write two bytes address then read data
    result = iic.Transfer(I2C_ADDR, (uint8_t*)&addr, sizeof(addr), rx_buf_ptr, size);
  }

  return result;
}

// *****************************************************************************
// ***   Public: Write   *******************************************************
// *****************************************************************************
Result Eeprom24::Write(uint16_t addr, uint8_t* tx_buf_ptr, uint16_t size)
{
  Result result = Result::RESULT_OK;

  // Check input parameters
  if(tx_buf_ptr == nullptr)
  {
    // Set error
    result = Result::ERR_NULL_PTR;
  }
  else if(addr + size > size_bytes)
  {
    // Set error
    result = Result::ERR_INVALID_SIZE;
  }
  else
  {
    // Disable write protection
    if(write_protection != nullptr)
    {
      write_protection->SetLow();
    }
    // Cycle for write pages
    while(size && result.IsGood())
    {
      // Get data size
      uint8_t data_size = size < page_size_bytes ? size : page_size_bytes;
      // For the first page
      if((addr % page_size_bytes) != 0U)
      {
        // Calculate data size from start address to the end of current page
        data_size = page_size_bytes - (addr % page_size_bytes);
        // If size less than remaining page bytes - use size
        data_size = size < data_size ? size : data_size;
      }
      // Decrease number of remaining bytes
      size -= data_size;
      // Store address
      buf[0U] = (addr >> 8) & 0xFF; // MSB
      buf[1U] = addr & 0xFF; // LSB
      // Increment address for next transaction
      addr += data_size;
      // Copy data
      memcpy(buf + 2U, tx_buf_ptr, data_size);
      // Transfer
      result = iic.Write(I2C_ADDR, buf, 2U + data_size);

      // Wait until writing finished
      if(result.IsGood())
      {
        // Check device response
        result = iic.IsDeviceReady(I2C_ADDR);
        // Clear repetition counter for tracking timeout
        repetition_cnt = 0U;
        // Wait until write operation finished
        while(result.IsBad())
        {
          // Delay 1 ms for start writing
          RtosTick::DelayMs(1U);
          // Check is device ready
          result = iic.IsDeviceReady(I2C_ADDR);
          // Check timeout
          if(repetition_cnt > WRITING_TIMEOUT_MS)
          {
            result = Result::ERR_I2C_TIMEOUT;
            break;
          }
        }
      }
    }
    // Enable write protection
    if(write_protection != nullptr)
    {
      write_protection->SetHigh();
    }
  }

  return result;
}

