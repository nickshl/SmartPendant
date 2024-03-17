//******************************************************************************
//  @file Mlx90614.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: Bosch BME280 library, implementation
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
#include "Mlx90614.h"

// *****************************************************************************
// ***   Initialize   **********************************************************
// *****************************************************************************
Result Mlx90614::Initialize(uint8_t addr)
{
  Result result = Result::RESULT_OK;

  // Save I2C sensor address
  i2c_addr = addr;

  // Enable I2C bus
  result = iic.Enable();

  // If successful
  if(result.IsGood())
  {
    // Clear temperature for clear upper bits
    uint16_t config_reg = 0U; // 1 0 011 111 0 1 11 0 101 - Ryobi IR sensor
    // Read Configuration Register
    result = ReadReg(EEPROM_CONFIG, config_reg);
  }

  // TODO: read data from sensor

  return result;
}

// ******************************************************************************
// ***   GetAmbientTemperature_x100   *******************************************
// ******************************************************************************
Result Mlx90614::GetAmbientTemperature_x100(int32_t& temp_x100)
{
  Result result = Result::RESULT_OK;

  // Clear temperature for clear upper bits
  temp_x100 = 0;
  // Read object temperature
  result = ReadReg(RAM_TA, *((uint16_t*)&temp_x100));
  // Normalize temperature
  temp_x100 *= 2; // Resolution is 0.02 degree C
  temp_x100 -= 27315; // Result in Kelvins, so convert it to Celsius

  return result;
}

// ******************************************************************************
// ***   GetObjectTemperature_x100   ********************************************
// ******************************************************************************
Result Mlx90614::GetObjectTemperature_x100(int32_t& temp_x100, ObjectType obj)
{
  Result result = Result::RESULT_OK;

  // Clear temperature for clear upper bits
  temp_x100 = 0;
  // Read object temperature
  result = ReadReg((obj == OBJECT1) ? RAM_TOBJ1 : RAM_TOBJ2, *((uint16_t*)&temp_x100));
  // Normalize temperature
  temp_x100 *= 2; // Resolution is 0.02 degree C
  temp_x100 -= 27315; // Result in Kelvins, so convert it to Celsius

  return result;
}

// ******************************************************************************
// ***   Read register value(16-bit unsigned)   *********************************
// ******************************************************************************
Result Mlx90614::ReadReg(uint8_t reg, uint16_t& value)
{
  Result result = Result::RESULT_OK;

  // Storage for read bytes
  uint16_t data[2] = {0};
  // Read data
  result = iic.Transfer(i2c_addr, &reg, sizeof(reg), (uint8_t*)data, 3U);

  // If the read succeeded, check CRC
  if(result.IsGood())
  {
    // Calculate CRC8
    uint8_t crc = 0U;
    crc = Crc8((i2c_addr << 1U), crc);        // Slave address with Write operation
    crc = Crc8(reg, crc);                     // Register to read
    crc = Crc8((i2c_addr << 1U) | 0x01, crc); // Slave address with Read operation
    crc = Crc8(((uint8_t*)data)[0], crc);     // First received byte
    crc = Crc8(((uint8_t*)data)[1], crc);     // Second received byte
    // Check CRC
    if(crc != (data[1] & 0x00FF))
    {
      result = Result::ERR_BAD_CRC;
    }
  }

  // If the read succeeded, check error flag
  if(result.IsGood())
  {
    if(data[0U] & 0x8000)
    {
      result = Result::ERR_BUSY;
    }
  }

  // If the read succeeded, store result
  if(result.IsGood())
  {
    value = data[0U];
  }

  return result;
}

// ******************************************************************************
// ***   Write register value(16-bit signed)   **********************************
// ******************************************************************************
Result Mlx90614::WriteReg(uint8_t reg, uint16_t value)
{
  Result result = Result::RESULT_OK;

  uint8_t buf[4];
  buf[0] = reg;
  buf[1] = ((uint8_t*)&value)[0];
  buf[2] = ((uint8_t*)&value)[1];
  buf[3] = Crc8((i2c_addr << 1U), 0U); // Slave address with Write operation
  buf[3] = Crc8(buf[0], buf[3]);       // Register to read
  buf[3] = Crc8(buf[1], buf[3]);       // First received byte
  buf[3] = Crc8(buf[2], buf[3]);       // Second received byte
  result = iic.Write(i2c_addr, buf, sizeof(buf));

  return result;
}


// ******************************************************************************
// ***   Calculate CRC 8 for SMBus   ********************************************
// ******************************************************************************
uint8_t Mlx90614::Crc8(uint8_t data, uint8_t crc)
{
  crc ^= data;

  for(uint8_t i = 0U; i < 8U; i++)
  {
    crc = (crc & 0x80U) ? ((crc << 1U) ^ 0x07U) : (crc << 1U); // Polynomial is X8+X2+X1+1
  }

  return crc;
}
