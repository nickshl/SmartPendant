//******************************************************************************
//  @file BoschBME280.cpp
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
#include "BoschBME280.h"

// *****************************************************************************
// ***   Initialize   **********************************************************
// *****************************************************************************
Result BoschBME280::Initialize(uint8_t addr)
{
  Result result = Result::RESULT_OK;

  // Save I2C sensor address
  i2c_addr = addr;

  result = iic.Enable();

  if(result.IsGood())
  {
    // Read Chip ID
    result = Read8(BME280_REGISTER_CHIPID, sensor_id);
    // Check if Chip ID is correct
    if(result.IsGood() && (sensor_id != 0x60))
    {
      result = Result::ERR_I2C_GENERAL;
    }
  }

  if(result.IsGood())
  {
    // Reset the device
    Write8(BME280_REGISTER_SOFTRESET, 0xB6);
  }

  if(result.IsGood())
  {
    // Wait for chip to wake up
    RtosTick::DelayMs(5U);
  }

  if(result.IsGood())
  {
    // If chip is still reading calibration
    while((result = IsReadingCalibration()) == Result::ERR_BUSY)
    {
      // Wait for chip to wake up
      RtosTick::DelayMs(1U);
    }
  }

  if(result.IsGood())
  {
    // Read trimming parameters, see DS 4.2.2
    result = ReadCoefficients();
  }

  if(result.IsGood())
  {
    // Set default sampling values
    result = SetSampling();
  }

  if(result.IsGood())
  {
    RtosTick::DelayMs(100U);
  }

  return result;
}

// ******************************************************************************
// ***   SetSampling   **********************************************************
// ******************************************************************************
Result BoschBME280::SetSampling(SensorModeType      mode,
                                SensorSamplingType  temperature_sampling,
                                SensorSamplingType  pressurre_sampling,
                                SensorSamplingType  humidity_sampling,
                                SensorFilterType    filter,
                                StandbyDurationType duration)
{
  Result result = Result::RESULT_OK;

  config_reg.spi3w_en = 0U;
  config_reg.reserved = 0U;
  config_reg.filter   = filter;
  config_reg.t_sb     = duration;
  result |= Write8(BME280_REGISTER_CONFIG, reinterpret_cast<uint8_t const&>(config_reg));

  // REGISTER_CONTROL should be set after setting the CONTROLHUMID register,
  // otherwise the values will not be applied (see datasheet 5.4.3)
  ctrl_hum_reg.reserved = 0U;
  ctrl_hum_reg.osrs_h   = humidity_sampling;
  result |= Write8(BME280_REGISTER_CONTROLHUMID, reinterpret_cast<uint8_t const&>(ctrl_hum_reg));

  ctrl_meas_reg.mode   = mode;
  ctrl_meas_reg.osrs_t = temperature_sampling;
  ctrl_meas_reg.osrs_p = pressurre_sampling;
  result |= Write8(BME280_REGISTER_CONTROL, reinterpret_cast<uint8_t const&>(ctrl_meas_reg));

  return result;
}

// ******************************************************************************
// ***   TakeMeasurement   ******************************************************
// ******************************************************************************
Result BoschBME280::TakeMeasurement()
{
  Result result = Result::RESULT_OK;

  if(ctrl_meas_reg.mode == MODE_FORCED)
  {
    // set to forced mode, i.e. "take next measurement"
    Write8(BME280_REGISTER_CONTROL, reinterpret_cast<uint8_t const&>(ctrl_meas_reg));
    // Variable
    uint8_t status = 0U;
    // Read status
    result = Read8(BME280_REGISTER_STATUS, status);
    // Wait until measurement has been completed
    while(result.IsGood() && (status & 0x08))
    {
      RtosTick::DelayMs(1U);
      // Read status
      result = Read8(BME280_REGISTER_STATUS, status);
    }
  }

  // Read RAW values
  if(result.IsGood())
  {
    uint8_t addr = BME280_REGISTER_PRESSUREDATA;
    // Local variable for read data(24 bit + 24 bit + 16 bit)
    uint8_t array[3U+3U+2U] = {0U};
    // Read all registers at once
    result = iic.Transfer(i2c_addr, &addr, sizeof(addr), array, sizeof(array));
    // Create RAW ADC data
    if(result.IsGood())
    {
      // Pressure
      adc_pressure = 0;
      result = ReverseArray((uint8_t*)&adc_pressure, &array[0U], 3U);
      adc_pressure >>= 4;
      // Temperature
      adc_temperature = 0;
      result |= ReverseArray((uint8_t*)&adc_temperature, &array[3U], 3U);
      adc_temperature >>= 4;
      // Humidity
      adc_humidity = 0;
      result |= ReverseArray((uint8_t*)&adc_humidity, &array[3U+3U], 2U);
      // Calculate t_fine for calculation Pressure & Humidity
      (void) GetTemperature_x100();
    }
  }

  return result;
}

// ******************************************************************************
// ***   GetTemperature_x100   **************************************************
// ******************************************************************************
int32_t BoschBME280::GetTemperature_x100(void)
{
  int32_t temp_x100 = 0;
  int32_t var1, var2;

  int32_t adc = adc_temperature;

  if (adc != 0x800000)
  {
    var1 = (((adc >> 3) - ((int32_t)bme280_calibration.dig_t1 << 1)) * ((int32_t)bme280_calibration.dig_t2)) >> 11;

    var2 = (((((adc >> 4) - ((int32_t)bme280_calibration.dig_t1)) * ((adc >> 4) - ((int32_t)bme280_calibration.dig_t1))) >> 12) * ((int32_t)bme280_calibration.dig_t3)) >> 14;

    t_fine = var1 + var2;

    temp_x100 = (t_fine * 5 + 128) >> 8;
  }

  return temp_x100;
}

// ******************************************************************************
// ***   GetPressure_x256   *****************************************************
// ******************************************************************************
int32_t BoschBME280::GetPressure_x256(void)
{
  int64_t var1, var2, press_x256 = 0;

  int32_t adc = adc_pressure;
  if(adc != 0x800000)
  {
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)bme280_calibration.dig_p6;
    var2 = var2 + ((var1 * (int64_t)bme280_calibration.dig_p5) << 17);
    var2 = var2 + (((int64_t)bme280_calibration.dig_p4) << 35);
    var1 = ((var1 * var1 * (int64_t)bme280_calibration.dig_p3) >> 8) + ((var1 * (int64_t)bme280_calibration.dig_p2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)bme280_calibration.dig_p1) >> 33;

    if (var1 == 0)
    {
      press_x256 = 0; // Avoid exception caused by division by zero
    }
    else
    {
      press_x256 = 1048576 - adc;
      press_x256 = (((press_x256 << 31) - var2) * 3125) / var1;
      var1 = (((int64_t)bme280_calibration.dig_p9) * (press_x256 >> 13) * (press_x256 >> 13)) >> 25;
      var2 = (((int64_t)bme280_calibration.dig_p8) * press_x256) >> 19;
      // Result
      press_x256 = ((press_x256 + var1 + var2) >> 8) + (((int64_t)bme280_calibration.dig_p7) << 4);
    }
  }

  return (int32_t)press_x256;
}

// ******************************************************************************
// ***   GetHumidity_x1024   ****************************************************
// ******************************************************************************
int32_t BoschBME280::GetHumidity_x1024(void)
{
    int32_t adc = adc_humidity;
    int32_t v_x1_u32r = 0;

    // value in case humidity measurement was disabled
    if(adc != 0x8000)
    {
      v_x1_u32r = (t_fine - ((int32_t)76800));

      v_x1_u32r = (((((adc << 14) - (((int32_t)bme280_calibration.dig_h4) << 20) -
                      (((int32_t)bme280_calibration.dig_h5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
                   (((((((v_x1_u32r * ((int32_t)bme280_calibration.dig_h6)) >> 10) *
                        (((v_x1_u32r * ((int32_t)bme280_calibration.dig_h3)) >> 11) + ((int32_t)32768))) >> 10) +
                      ((int32_t)2097152)) * ((int32_t)bme280_calibration.dig_h2) + 8192) >> 14));

      v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)bme280_calibration.dig_h1)) >> 4));

      v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
      v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    }

    return (v_x1_u32r >> 12);
}

// ******************************************************************************
// ***   ReadCoefficients   *****************************************************
// ******************************************************************************
Result BoschBME280::ReadCoefficients(void)
{
  Result result = Result::RESULT_OK;

  result |= Read16(BME280_REGISTER_DIG_T1, bme280_calibration.dig_t1);
  result |= Read16(BME280_REGISTER_DIG_T2, bme280_calibration.dig_t2);
  result |= Read16(BME280_REGISTER_DIG_T3, bme280_calibration.dig_t3);

  result |= Read16(BME280_REGISTER_DIG_P1, bme280_calibration.dig_p1);
  result |= Read16(BME280_REGISTER_DIG_P2, bme280_calibration.dig_p2);
  result |= Read16(BME280_REGISTER_DIG_P3, bme280_calibration.dig_p3);
  result |= Read16(BME280_REGISTER_DIG_P4, bme280_calibration.dig_p4);
  result |= Read16(BME280_REGISTER_DIG_P5, bme280_calibration.dig_p5);
  result |= Read16(BME280_REGISTER_DIG_P6, bme280_calibration.dig_p6);
  result |= Read16(BME280_REGISTER_DIG_P7, bme280_calibration.dig_p7);
  result |= Read16(BME280_REGISTER_DIG_P8, bme280_calibration.dig_p8);
  result |= Read16(BME280_REGISTER_DIG_P9, bme280_calibration.dig_p9);

  result |= Read8(BME280_REGISTER_DIG_H1, bme280_calibration.dig_h1);
  result |= Read16(BME280_REGISTER_DIG_H2, bme280_calibration.dig_h2);
  result |= Read8(BME280_REGISTER_DIG_H3, bme280_calibration.dig_h3);

  // Variables for store values
  uint8_t dig_h4;
  uint8_t dig_h45;
  uint8_t dig_h5;
  // Read values
  result |= Read8(BME280_REGISTER_DIG_H4, dig_h4);
  result |= Read8(BME280_REGISTER_DIG_H45, dig_h45);
  result |= Read8(BME280_REGISTER_DIG_H5, dig_h5);
  // Make digit from values
  bme280_calibration.dig_h4 = (dig_h4 << 4) | (dig_h45 & 0xF);
  bme280_calibration.dig_h5 = (dig_h5 << 4) | (dig_h45 >> 4);

  result |= Read8(BME280_REGISTER_DIG_H6, bme280_calibration.dig_h6);

  return result;
}

// ******************************************************************************
// ***   Check is chip still reading calibration data   *************************
// ******************************************************************************
Result BoschBME280::IsReadingCalibration(void)
{
  Result result = Result::RESULT_OK;

  uint8_t status = 0U;
  // Read status
  result = Read8(BME280_REGISTER_STATUS, status);
  // Check result
  if(result.IsGood())
  {
    if((status & 1U) != 0U)
    {
      result = Result::ERR_BUSY;
    }
  }

  return result;
}

// ******************************************************************************
// ***   Write register value(8-bit)   ******************************************
// ******************************************************************************
Result BoschBME280::Write8(uint8_t reg, uint8_t value)
{
  uint8_t buf[2];
  buf[0] = reg;
  buf[1] = value;
  return iic.Write(i2c_addr, buf, sizeof(buf));
}

// ******************************************************************************
// ***   Read register value(8-bit unsigned)   **********************************
// ******************************************************************************
Result BoschBME280::Read8(uint8_t reg, uint8_t& value)
{
  return iic.Transfer(i2c_addr, &reg, sizeof(reg), &value, sizeof(value));
}

// ******************************************************************************
// ***   Read register value(8-bit signed)   ************************************
// ******************************************************************************
Result BoschBME280::Read8(uint8_t reg, int8_t& value)
{
  return iic.Transfer(i2c_addr, &reg, sizeof(reg), (uint8_t*)&value, sizeof(value));
}

// ******************************************************************************
// ***   Read register value(16-bit unsigned)   *********************************
// ******************************************************************************
Result BoschBME280::Read16(uint8_t reg, uint16_t& value, bool reverse)
{
  Result result = Result::RESULT_OK;

  // Read data
  result = iic.Transfer(i2c_addr, &reg, sizeof(reg), (uint8_t*)&value, sizeof(value));
  // Change endian if needed
  if(reverse)
  {
    value = (value >> 8) | (value << 8);
  }

  return result;
}

// ******************************************************************************
// ***   Read register value(16-bit signed)   ***********************************
// ******************************************************************************
Result BoschBME280::Read16(uint8_t reg, int16_t& value, bool reverse)
{
  Result result = Result::RESULT_OK;

  // Read data
  result = iic.Transfer(i2c_addr, &reg, sizeof(reg), (uint8_t*)&value, sizeof(value));
  // Change endian if needed
  if(reverse)
  {
    value = (value >> 8) | (value << 8);
  }

  return result;
}

// ******************************************************************************
// ***   Reverse byte order in array   ******************************************
// ******************************************************************************
Result BoschBME280::ReverseArray(uint8_t* dst, uint8_t* src, uint32_t size)
{
  Result result = Result::ERR_NULL_PTR;

  if((dst != nullptr) && (src != nullptr))
  {
    for(uint32_t i = 0U; i < size; i++)
    {
      dst[i] = src[size - i - 1U];
    }
    result = Result::RESULT_OK;
  }

  return result;
}
