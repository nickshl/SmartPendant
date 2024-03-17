//******************************************************************************
//  @file BoschBME280.h
//  @author Nicolai Shlapunov
//
//  @details Bosch BME280: Library, header
//
//  @section LICENSE
//
//   Software License Agreement (Modified BSD License)
//
//   Copyright (c) 2018, Devtronic & Nicolai Shlapunov
//   All rights reserved.
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//   3. Neither the name of the Devtronic nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//   4. Redistribution and use of this software other than as permitted under
//      this license is void and will automatically terminate your rights under
//      this license.
//
//   THIS SOFTWARE IS PROVIDED BY DEVTRONIC ''AS IS'' AND ANY EXPRESS OR IMPLIED
//   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL DEVTRONIC BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
//   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  @section SUPPORT
//
//   Devtronic invests time and resources providing this open source code,
//   please support Devtronic and open-source hardware/software by
//   donations and/or purchasing products from Devtronic.
//
//******************************************************************************

#ifndef BoschBME280_h
#define BoschBME280_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "IIic.h"

// *****************************************************************************
// ***   Bosch BME280 library  *************************************************
// *****************************************************************************
class BoschBME280
{
  public:
    // *************************************************************************
    // ***   Sampling rates   **************************************************
    // *************************************************************************
    enum SensorSamplingType
    {
      SAMPLING_NONE = 0x00, // 000 = skipped
      SAMPLING_X1,          // 001 = x1
      SAMPLING_X2,          // 010 = x2
      SAMPLING_X4,          // 011 = x4
      SAMPLING_X8,          // 100 = x8
      SAMPLING_X16          // 101 and above = x16
    };

    // *************************************************************************
    // ***   Power modes   *****************************************************
    // *************************************************************************
    enum SensorModeType
    {
      MODE_SLEEP  = 0x00, // 00       = sleep
      MODE_FORCED = 0x01, // 01 or 10 = forced
      MODE_NORMAL = 0x11  // 11       = normal
    };

    // *************************************************************************
    // ***   Filter values   ***************************************************
    // *************************************************************************
    enum SensorFilterType
    {
      FILTER_OFF = 0x00, // 000 = filter off
      FILTER_X2,         // 001 = x2 filter
      FILTER_X4,         // 010 = x4 filter
      FILTER_X8,         // 011 = x8 filter
      FILTER_X16,        // 100 and above = 16x filter
    };

    // *************************************************************************
    // ***   Standby duration   ************************************************
    // *************************************************************************
    enum StandbyDurationType
    {
      STANDBY_MS_0_5  = 0x00, // 000 =    0.5 ms
      STANDBY_MS_10   = 0x06, // 110 =   10 ms
      STANDBY_MS_20   = 0x07, // 111 =   20 ms
      STANDBY_MS_62_5 = 0x01, // 001 =   62.5 ms
      STANDBY_MS_125  = 0x02, // 010 =  125 ms
      STANDBY_MS_250  = 0x03, // 100 =  500 ms
      STANDBY_MS_500  = 0x04, // 101 = 1000 ms
      STANDBY_MS_1000 = 0x05  // 011 =  250 ms
    };

    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    BoschBME280(IIic& iic_ref) : iic(iic_ref) {};

    // *************************************************************************
    // ***   Initialize   ******************************************************
    // *************************************************************************
    Result Initialize(uint8_t addr = BME280_ADDRESS);

    // *************************************************************************
    // ***   SetSampling   *****************************************************
    // *************************************************************************
    Result SetSampling(SensorModeType mode                     = MODE_NORMAL,
                       SensorSamplingType temperature_sampling = SAMPLING_X16,
                       SensorSamplingType pressure_sampling    = SAMPLING_X16,
                       SensorSamplingType humidity_sampling    = SAMPLING_X16,
                       SensorFilterType filter                 = FILTER_OFF,
                       StandbyDurationType duration            = STANDBY_MS_0_5);

    // *************************************************************************
    // ***   TakeMeasurement   *************************************************
    // *************************************************************************
    Result TakeMeasurement();

    // *************************************************************************
    // ***   GetTemperature_x100   *********************************************
    // *************************************************************************
    int32_t GetTemperature_x100(void);

    // *************************************************************************
    // ***   GetPressure_x256   ************************************************
    // *************************************************************************
    int32_t GetPressure_x256(void);

    // *************************************************************************
    // ***   GetHumidity_x1024   ***********************************************
    // *************************************************************************
    int32_t GetHumidity_x1024(void);

    // *************************************************************************
    // ***   GetTemperature   **************************************************
    // *************************************************************************
    inline float GetTemperature(void) {return ((float)GetTemperature_x100() / 100.0F);}

    // *************************************************************************
    // ***   GetPressure   *****************************************************
    // *************************************************************************
    inline float GetPressure(void) {return ((float)GetPressure_x256() / 256.0F);}

    // *************************************************************************
    // ***   GetHumidity   *****************************************************
    // *************************************************************************
    inline float GetHumidity(void) {return ((float)GetHumidity_x1024() / 1024.0F);}

  private:

    // ***   default I2C address **********************************************
    static const uint8_t BME280_ADDRESS = 0x76;

    // ***   Register addresses   *********************************************
    enum Registers
    {
      BME280_REGISTER_DIG_T1       = 0x88,
      BME280_REGISTER_DIG_T2       = 0x8A,
      BME280_REGISTER_DIG_T3       = 0x8C,

      BME280_REGISTER_DIG_P1       = 0x8E,
      BME280_REGISTER_DIG_P2       = 0x90,
      BME280_REGISTER_DIG_P3       = 0x92,
      BME280_REGISTER_DIG_P4       = 0x94,
      BME280_REGISTER_DIG_P5       = 0x96,
      BME280_REGISTER_DIG_P6       = 0x98,
      BME280_REGISTER_DIG_P7       = 0x9A,
      BME280_REGISTER_DIG_P8       = 0x9C,
      BME280_REGISTER_DIG_P9       = 0x9E,

      BME280_REGISTER_DIG_H1       = 0xA1,
      BME280_REGISTER_DIG_H2       = 0xE1,
      BME280_REGISTER_DIG_H3       = 0xE3,
      BME280_REGISTER_DIG_H4       = 0xE4,
      BME280_REGISTER_DIG_H45      = 0xE5,
      BME280_REGISTER_DIG_H5       = 0xE6,
      BME280_REGISTER_DIG_H6       = 0xE7,

      BME280_REGISTER_CHIPID       = 0xD0,
      BME280_REGISTER_SOFTRESET    = 0xE0,

      BME280_REGISTER_CONTROLHUMID = 0xF2,
      BME280_REGISTER_STATUS       = 0XF3,
      BME280_REGISTER_CONTROL      = 0xF4,
      BME280_REGISTER_CONFIG       = 0xF5,
      BME280_REGISTER_PRESSUREDATA = 0xF7,
      BME280_REGISTER_TEMPDATA     = 0xFA,
      BME280_REGISTER_HUMIDDATA    = 0xFD
    };

    // Reference to I2C interface
    IIic& iic;
    // I2C address
    uint8_t i2c_addr = BME280_ADDRESS;
    // Sensor ID
    uint8_t sensor_id = 0;

    // Some data for calculate pressure & humidity
    int32_t t_fine = 0;
    // Variables for store raw values
    int32_t adc_temperature = 0;
    int32_t adc_pressure = 0;
    uint16_t adc_humidity = 0;

    // ***   Calibration data structure   **************************************
    struct CalibrationData
    {
      // Temperature compensation values
      uint16_t dig_t1;
      int16_t  dig_t2;
      int16_t  dig_t3;

      // Pressure compensation values
      uint16_t dig_p1;
      int16_t  dig_p2;
      int16_t  dig_p3;
      int16_t  dig_p4;
      int16_t  dig_p5;
      int16_t  dig_p6;
      int16_t  dig_p7;
      int16_t  dig_p8;
      int16_t  dig_p9;

      // Humidity compensation values
      uint8_t  dig_h1;
      int16_t  dig_h2;
      uint8_t  dig_h3;
      int16_t  dig_h4;
      int16_t  dig_h5;
      int8_t   dig_h6;
    };
    CalibrationData bme280_calibration;

    // ***   Configuration register structure   *********************************
    struct Config
    {
      uint8_t spi3w_en : 1;
      uint8_t reserved : 1;
      uint8_t filter   : 3; // Filter settings
      uint8_t t_sb     : 3; // Inactive duration (StandBy time) in normal mode
    };
    Config config_reg;

    // ***   Measurement control register   *************************************
    struct CtrlMeas
    {
      uint8_t mode   : 2; // Device mode
      uint8_t osrs_p : 3; // Pressure oversampling
      uint8_t osrs_t : 3; // Temperature oversampling
    };
    CtrlMeas ctrl_meas_reg;

    // ***   Humidity control register structure   *****************************
    struct CtrlHum
    {
      uint8_t osrs_h   : 3;
      uint8_t reserved : 5;
    };
    CtrlHum ctrl_hum_reg;

    // *************************************************************************
    // ***   Read Coefficients   ***********************************************
    // *************************************************************************
    Result ReadCoefficients(void);

    // *************************************************************************
    // ***   Check is chip still reading calibration data   ********************
    // *************************************************************************
    Result IsReadingCalibration(void);

    // *************************************************************************
    // ***   Write register value(8-bit)   *************************************
    // *************************************************************************
    Result Write8(uint8_t reg, uint8_t value);

    // *************************************************************************
    // ***   Read register value(8-bit unsigned)   *****************************
    // *************************************************************************
    Result Read8(uint8_t reg, uint8_t& value);

    // *************************************************************************
    // ***   Read register value(8-bit signed)   *******************************
    // *************************************************************************
    Result Read8(uint8_t reg, int8_t& value);

    // *************************************************************************
    // ***   Read register value(16-bit unsigned)   ****************************
    // *************************************************************************
    Result Read16(uint8_t reg, uint16_t& value, bool reverse = false);

    // *************************************************************************
    // ***   Read register value(16-bit signed)   ******************************
    // *************************************************************************
    Result Read16(uint8_t reg, int16_t& value, bool reverse = false);

    // *************************************************************************
    // ***   Reverse byte order in array   *************************************
    // *************************************************************************
    Result ReverseArray(uint8_t* dst, uint8_t* src, uint32_t size);

    // *************************************************************************
    // ***   Private: Constructors and assign operator - prevent copying   *****
    // *************************************************************************
    BoschBME280();
    BoschBME280(const BoschBME280&);
    BoschBME280& operator=(const BoschBME280);
};

#endif
