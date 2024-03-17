//******************************************************************************
//  @file FT6236.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: FT6236 Low Level Driver Class, implementation
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
#include "FT6236.h"

// *****************************************************************************
// ***   Public: Init touchscreen   ********************************************
// *****************************************************************************
Result FT6236::Init(void)
{
  Result result = Result::RESULT_OK;

  // Timeout shouln't be less than 2. One mean that timeout can be anywhere between
  // 0 and 1 ms, in some cases it cause to fail by timeout. 2 mean that timeout
  // will be between 1 and 2 ms which should be enough for byte transfer.
  iic.SetTxTimeout(2u);
  iic.SetRxTimeout(2u);

  result = iic.Enable();

  if(result.IsGood())
  {
    // Adjust threshold
    result = WriteReg8(REG_THRESHOLD, DEFAULT_THRESHOLD);
  }

  if(result.IsGood())
  {
    // Set control mode: 0 - keep the Active mode when there is no touching
    //                   1 - switching from Active mode to Monitor mode automatically when there is no touching (default)
    result = WriteReg8(REG_CTRL, 0u);
  }

  if(result.IsGood())
  {
    uint8_t reg = 0u;
    // Read register
    result = ReadReg8(REG_FOCALTECH_ID, reg);
    // Check if our chip has the correct Vendor ID
    if(reg != FOCALTECH_ID)
    {
        result = Result::ERR_INVALID_ITEM;
    }
  }
  
  if(result.IsGood())
  {
    uint8_t id = 0u;
    // Read register
    result = ReadReg8(REG_CHIPER, id);
    // Check if our chip has the correct Chip ID.
    if((id != FT6236_CHIPER) && (id != FT6236U_CHIPER) && (id != FT6206_CHIPER))
    {
      result = Result::ERR_INVALID_ITEM;
    }
  }

  // Always Ok
  return result;
}

// *****************************************************************************
// ***   Public: If touched - return true.   ***********************************
// *****************************************************************************
bool FT6236::IsTouched(void)
{
  uint8_t n = 0u;
  // Read number of touches
  Result result = ReadReg8(REG_TD_STATUS, n);
  // In case something goes wrong - reset I2C
  if(result.IsBad())
  {
    // TODO: figure out why sometimes I2C fails. It start happens when timer
    // task priority risen.
    iic.Reset();
  }
  // Only 1-2 is valid
  if((n > 2) || (result != Result::RESULT_OK))
  {
    n = 0;
  }
  // Return result
  return n;
}

// *****************************************************************************
// ***   Public: Get X and Y coordinates. If touched - return true.   **********
// *****************************************************************************
bool FT6236::GetRawXY(int32_t& x, int32_t& y)
{
  // Return value
  bool ret = false;
  // If touch properly initialized and actual touch present
  if(IsTouched())
  {
    uint8_t i2cdat[16];

    // Register to read
    uint8_t reg = 0x00u;
    // Read 16 bytes
    if(iic.Transfer(TOUCH_I2C_ADDR, &reg, sizeof(reg), i2cdat, sizeof(i2cdat)).IsGood())
    {
      // Save number of touches
      touch_cnt = i2cdat[REG_TD_STATUS];
      // Can't be more than 2
      if(touch_cnt > 2u)
      {
        touch_cnt = 0u;
      }

      // Save touches
      for(uint8_t i = 0u; i < 2u; i++)
      {
        touch_x[i] = (i2cdat[0x03u + i * 6u] & 0x0Fu) << 8u;
        touch_x[i] |= i2cdat[0x04u + i * 6u];
        touch_y[i] = (i2cdat[0x05u + i * 6u] & 0x0Fu) << 8u;
        touch_y[i] |= i2cdat[0x06u + i * 6u];
        touch_id[i] = i2cdat[0x05u + i * 6u] >> 4u;
      }

      // Return first touch
      x = touch_x[0u];
      y = touch_y[0u];

      // Touch present
      ret = true;
    }
    else // In case something goes wrong - reset I2C
    {
      // TODO: figure out why sometimes I2C fails. It start happens when timer
      // task priority risen.
      iic.Reset();
    }
  }
  // Return result
  return ret;
}

// *****************************************************************************
// ***   Public: Get X and Y coordinates. If touched - return true.   **********
// *****************************************************************************
bool FT6236::GetXY(int32_t& x, int32_t& y)
{
  // Get coordinates
  bool touched = GetRawXY(x, y);
  // Convert raw coordinates to coorinates including rotation
  if(touched)
  {
    int32_t tmp = 0u;

    switch(rotation)
    {
      case ITouchscreen::ROTATION_BOTTOM:
        x = width - x;
        y = height - y;
        break;

      case ITouchscreen::ROTATION_RIGHT:
        tmp = y;
        y = width - x;
        x = tmp;
        break;

      case ITouchscreen::ROTATION_LEFT:
        tmp = x;
        x = height - y;
        y = tmp;
        break;

      case ITouchscreen::ROTATION_TOP: // Intational fall trough - for TOP rotation we should't do anythyng
      default:
        break;
    }
  }
  // Return touch state
  return touched;
}

// *****************************************************************************
// ***   Private: WriteReg8   **************************************************
// *****************************************************************************
Result FT6236::WriteReg8(uint8_t reg, uint8_t value)
{
  uint8_t buf[2];
  buf[0] = reg;
  buf[1] = value;
  return iic.Write(TOUCH_I2C_ADDR, buf, sizeof(buf));
}

// *****************************************************************************
// ***   Read register value(8-bit unsigned)   *********************************
// *****************************************************************************
Result FT6236::ReadReg8(uint8_t reg, uint8_t& value)
{
  return iic.Transfer(TOUCH_I2C_ADDR, &reg, sizeof(reg), &value, sizeof(value));
}
