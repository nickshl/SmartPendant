//******************************************************************************
//  @file Tcs34725.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: TCS34725 Sensor driver, implementation
//
//  @copyright Copyright (c) 2019, Devtronic & Nicolai Shlapunov
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
#include "Tcs34725.h"

#include "string.h" // for memcpy()

// *****************************************************************************
// ***   Public: Initialize   **************************************************
// *****************************************************************************
Result Tcs34725::Initialize()
{
  Result result = Result::RESULT_OK;
  iic.SetTxTimeout(10U);
  iic.SetRxTimeout(10U);

  result = iic.Enable();

  if(result.IsGood())
  {
    uint8_t data = 0x00;
    // Read Chip ID
    result = Read8(REGISTER_ID, data);
    if((data != 0x44) && (data != 0x4D)) // 0x44 = TCS34725, 0x4D = TCS34727
    {
      result = Result::ERR_I2C_GENERAL;
    }
  }

  if(result.IsGood())
  {
    // Set internal inited flag
    inited = true;
    // Set default integration time and gain
    SetIntegrationTime(integration_time);
    SetGain(gain);
    // At power up, the device initializes in a low-power Sleep state
    Enable();
  }

  return result;
}

// *****************************************************************************
// ***   Public: Enable   ******************************************************
// *****************************************************************************
Result Tcs34725::Enable(void)
{
  Result result = Result::RESULT_OK;

  // Read current register state
  uint8_t reg = 0;
  result = Read8(REGISTER_ENABLE, reg);
  // Turn on sensor
  if(result.IsGood())
  {
    result = Write8(REGISTER_ENABLE, reg | ENABLE_PON);
  }
  // Writing a 1 activates the RGBC
  if(result.IsGood())
  {
    RtosTick::DelayMs(3U); // There is a 2.4 ms warm-up delay if PON is enabled.
    result |= Write8(REGISTER_ENABLE, ENABLE_PON | ENABLE_AEN);
  }

  return result;
}

// *****************************************************************************
// ***   Public: Disable   *****************************************************
// *****************************************************************************
Result Tcs34725::Disable(void)
{
  Result result = Result::RESULT_OK;

  result = Write8(REGISTER_ENABLE, 0U);

  // Read current register state
  uint8_t reg = 0;
  result = Read8(REGISTER_ENABLE, reg);
  // Turn off sensor
  if(result.IsGood())
  {
    result = Write8(REGISTER_ENABLE, reg & ~(ENABLE_PON | ENABLE_AEN));
  }

  return result;
}

// *****************************************************************************
// ***   Public: SetGain   *****************************************************
// *****************************************************************************
Result Tcs34725::SetGain(GainType g)
{
  Result result = Result::ERR_BUSY;

  if(inited == true)
  {
    // Range check
    if(g >= GAIN_CNT) g = GAIN_60X;
    // Update register
    result = Write8(REGISTER_CONTROL, g);
    // Update value
    gain = g;
  }

  return result;
}

// *****************************************************************************
// ***   Public: IncGain   *****************************************************
// *****************************************************************************
Result Tcs34725::IncGain(void)
{
  Result result = Result::ERR_BUSY;

  if(gain < GAIN_60X)
  {
    result = SetGain((GainType)(gain + 1U));
  }

  return result;
}

// *****************************************************************************
// ***   Public: DecGain   *****************************************************
// *****************************************************************************
Result Tcs34725::DecGain(void)
{
  Result result = Result::ERR_BUSY;

  if(gain > GAIN_1X)
  {
    result = SetGain((GainType)(gain - 1U));
  }

  return result;
}

// *****************************************************************************
// ***   Public: GetGainValue   ************************************************
// *****************************************************************************
uint32_t Tcs34725::GetGainValue(GainType g)
{
  uint32_t gain_value = 0U;
  // Check for default value
  if(g == GAIN_CNT)
  {
    g = gain;
  }
  // Convert gain to a integer value
  switch(g)
  {
    case GAIN_60X:
      gain_value = 60U;
      break;

    case GAIN_16X:
      gain_value = 16U;
      break;

    case GAIN_4X:
      gain_value = 4U;
      break;

    case GAIN_1X: // Intentional fall-trough
    default:
      gain_value = 1U;
      break;
  }
  // Return result
  return gain_value;
}

// *****************************************************************************
// ***   Public: SetIntegrationTime   ******************************************
// *****************************************************************************
Result Tcs34725::SetIntegrationTime(IntegrationTimeType it)
{
  Result result = Result::ERR_BUSY;

  if(inited == true)
  {
    // Update register
    result = Write8(REGISTER_ATIME, it);
    // Update value
    integration_time = it;
  }

  return result;
}

// *****************************************************************************
// ***   Public: IsDataReady   *************************************************
// *****************************************************************************
Result Tcs34725::IsDataReady(bool& is_ready)
{
  Result result = Result::ERR_NULL_PTR;

  if(inited == true)
  {
    uint8_t data = 0x00;
    // Read Chip ID
    result = Read8(REGISTER_STATUS, data);
    if(data & STATUS_AVALID)
    {
      is_ready = true;
    }
    else
    {
      is_ready = false;
    }
  }

  return result;
}

// *****************************************************************************
// ***   Public: GetRawData   **************************************************
// *****************************************************************************
Result Tcs34725::GetRawData(uint16_t& r, uint16_t& g, uint16_t& b, uint16_t& c)
{
  Result result = Result::ERR_NULL_PTR;

  if(inited == true)
  {
    result =  Read16(REGISTER_CDATAL, c);
    result |= Read16(REGISTER_RDATAL, r);
    result |= Read16(REGISTER_GDATAL, g);
    result |= Read16(REGISTER_BDATAL, b);
    // If good - store to calculate color temperature in future
    if(result.IsGood())
    {
      red = r;
      green = g;
      blue = b;
      clear = c;
    }
  }

  return result;
}

// *****************************************************************************
// ***   Public: GetLux   ******************************************************
// *****************************************************************************
Result Tcs34725::GetLux(uint16_t& lux)
{
  Result result = Result::ERR_BUSY;

  if(inited == true)
  {
    // AMS RGB sensors have no direct IR channel and the IR content must be
    // calculated indirectly
    uint16_t ir = (red + green + blue > clear) ? (red + green + blue - clear) / 2 : 0;

    // Remove the IR component from the raw RGB values
    uint16_t r = red   - ir;
    uint16_t g = green - ir;
    uint16_t b = blue  - ir;

    // Calculate the counts per lux (CPL)
    float cpl = (((256 - integration_time) * 2.4f) * GetGainValue()) / (1.0f * 310.0f);

    // Finally, calculate the lux
    lux = (uint16_t)((0.136f * (float)r + 1.000f * (float)g - 0.444f * (float)b) / cpl);

    result = Result::RESULT_OK;
  }

  return result;
}

// *****************************************************************************
// ***   Public: GetColorTemperature   *****************************************
// *****************************************************************************
Result Tcs34725::GetColorTemperature(uint16_t& color_temperature)
{
  Result result = Result::ERR_BUSY;

  if(inited == true)
  {
    // AMS RGB sensors have no direct IR channel and the IR content must be
    // calculated indirectly
    uint16_t ir = (red + green + blue > clear) ? (red + green + blue - clear) / 2 : 0;

    // Remove the IR component from the raw RGB values
    uint16_t r = red  - ir;
    uint16_t b = blue - ir;

    // A simple method of measuring color temp is to use the ratio of blue
    // to red light, taking IR cancellation into account.
    color_temperature = (3810U * (uint32_t)b) / (uint32_t)r + 1391U;

    result = Result::RESULT_OK;
  }

  return result;
}

// *****************************************************************************
// ***   Public: SetLimits   ***************************************************
// *****************************************************************************
Result Tcs34725::SetLimits(uint16_t low, uint16_t high)
{
  Result result = Result::RESULT_OK;

  if(inited == true)
  {
    result =  Write8(REGISTER_AILTL, low & 0xFF);
    result |= Write8(REGISTER_AILTH, low >> 8);
    result |= Write8(REGISTER_AIHTL, high & 0xFF);
    result |= Write8(REGISTER_AIHTH, high >> 8);
  }

  return result;
}

// *****************************************************************************
// ***   Public: SetInterrupt   ************************************************
// *****************************************************************************
Result Tcs34725::SetInterrupt(bool enable)
{
  Result result = Result::RESULT_OK;

  if(inited == true)
  {
    // Read current register state
    uint8_t reg = 0U;
    result = Read8(REGISTER_ENABLE, reg);
    // Turn on sensor
    if(result.IsGood())
    {
      // Set or clear flag
      reg = (enable == true) ? (reg | ENABLE_AIEN) : (reg & ~ENABLE_AIEN);
      // Write new register value
      result = Write8(REGISTER_ENABLE, reg | ENABLE_PON);
    }
  }

  return result;
}

// *****************************************************************************
// ***   Public: ClearInterrupt   **********************************************
// *****************************************************************************
Result Tcs34725::ClearInterrupt(void)
{
  Result result = Result::RESULT_OK;

  if(inited == true)
  {
    // Magic value for clear channel interrupt clear
    uint8_t command = COMMAND_BIT | 0x66;
    // Write command
    result = iic.Write(i2c_addr, &command, sizeof(command));
  }

  return result;
}

// *****************************************************************************
// ***   Public: Read   ********************************************************
// *****************************************************************************
Result Tcs34725::Read(uint16_t addr, uint8_t* rx_buf_ptr, uint16_t size)
{
  Result result = Result::ERR_NULL_PTR;

  return result;
}

// *****************************************************************************
// ***   Public: Write   *******************************************************
// *****************************************************************************
Result Tcs34725::Write(uint16_t addr, uint8_t* tx_buf_ptr, uint16_t size)
{
  Result result = Result::ERR_NULL_PTR;

  return result;
}

// ******************************************************************************
// ***   Write register value(8-bit)   ******************************************
// ******************************************************************************
Result Tcs34725::Write8(uint8_t reg, uint8_t value)
{
  uint8_t buf[2];
  buf[0] = reg | COMMAND_BIT;
  buf[1] = value;
  return iic.Write(i2c_addr, buf, sizeof(buf));
}

// ******************************************************************************
// ***   Read register value(8-bit unsigned)   **********************************
// ******************************************************************************
Result Tcs34725::Read8(uint8_t reg, uint8_t& value)
{
  // Add command bit
  reg |= COMMAND_BIT;
  // Transfer & return result
  return iic.Transfer(i2c_addr, &reg, sizeof(reg), (uint8_t*)&value, sizeof(value));
}

// ******************************************************************************
// ***   Read register value(16-bit unsigned)   *********************************
// ******************************************************************************
Result Tcs34725::Read16(uint8_t reg, uint16_t& value, bool reverse)
{
  Result result = Result::RESULT_OK;

  // Add command bit
  reg |= COMMAND_BIT;
  // Read data
  result = iic.Transfer(i2c_addr, &reg, sizeof(reg), (uint8_t*)&value, sizeof(value));
  // Change endian if needed
  if(reverse)
  {
    value = (value >> 8) | (value << 8);
  }

  return result;
}
