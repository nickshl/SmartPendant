//******************************************************************************
//  @file Tcs34725.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: TCS34725 Sensor driver, header
//
//  @section LICENSE
//
//   Software License Agreement (Modified BSD License)
//
//   Copyright (c) 2019, Devtronic & Nicolai Shlapunov
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

#ifndef Tcs34725_h
#define Tcs34725_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "IIic.h"

// *****************************************************************************
// ***   EEPROM 24C*** Driver Class   ******************************************
// *****************************************************************************
class Tcs34725
{
  public:
    // *************************************************************************
    // ***   Public: Gain enum   ***********************************************
    // *************************************************************************
    enum GainType : uint8_t
    {
      GAIN_1X  = 0x00, // 1x
      GAIN_4X  = 0x01, // 4x
      GAIN_16X = 0x02, // 16x
      GAIN_60X = 0x03, // 60x
      GAIN_CNT
    };

    // *************************************************************************
    // ***   Public: Integration Time enum   ***********************************
    // *************************************************************************
    enum IntegrationTimeType : uint8_t
    {
      INTEGRATIONTIME_2_4MS = 0xFF, //   1 cycle,  2.4ms, Max Count: 1024
      INTEGRATIONTIME_24MS  = 0xF6, //  10 cycles,  24ms, Max Count: 10240
      INTEGRATIONTIME_50MS  = 0xEB, //  20 cycles,  50ms, Max Count: 20480
      INTEGRATIONTIME_101MS = 0xD5, //  42 cycles, 101ms, Max Count: 43008
      INTEGRATIONTIME_154MS = 0xC0, //  64 cycles, 154ms, Max Count: 65535
      INTEGRATIONTIME_700MS = 0x00  // 256 cycles, 700ms, Max Count: 65535
    };

    // *************************************************************************
    // ***   Public: Wait Time enum   ******************************************
    // *************************************************************************
    enum WaitTimeVType : uint8_t
    {
      WTIME_2_4MS = 0xFF, // WLONG0 = 2.4ms, WLONG1 = 0.029s
      WTIME_204MS = 0xAB, // WLONG0 = 204ms, WLONG1 = 2.45s
      WTIME_614MS = 0x00  // WLONG0 = 614ms, WLONG1 = 7.4s
    };

    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    explicit Tcs34725(IIic& iic_ref) : iic(iic_ref) {};

    // *************************************************************************
    // ***   Public: Initialize   **********************************************
    // *************************************************************************
    Result Initialize();

    // *************************************************************************
    // ***   Public: Enable   **************************************************
    // *************************************************************************
    Result Enable(void);

    // *************************************************************************
    // ***   Public: Enable   **************************************************
    // *************************************************************************
    Result Disable(void);

    // *************************************************************************
    // ***   Public: SetGain   *************************************************
    // *************************************************************************
    Result SetGain(GainType g);

    // *************************************************************************
    // ***   Public: IncGain   *************************************************
    // *************************************************************************
    Result IncGain(void);

    // *************************************************************************
    // ***   Public: DecGain   *************************************************
    // *************************************************************************
    Result DecGain(void);

    // *************************************************************************
    // ***   Public: GetGainValue   ********************************************
    // *************************************************************************
    uint32_t GetGainValue(GainType g = GAIN_CNT);

    // *************************************************************************
    // ***   Public: SetIntegrationTime   **************************************
    // *************************************************************************
    Result SetIntegrationTime(IntegrationTimeType it);

    // *************************************************************************
    // ***   Public: IsDataReady   *********************************************
    // *************************************************************************
    Result IsDataReady(bool& is_ready);

    // *************************************************************************
    // ***   Public: GetRawData   **********************************************
    // *************************************************************************
    Result GetRawData(uint16_t& r, uint16_t& g, uint16_t& b, uint16_t& c);

    // *************************************************************************
    // ***   Public: GetLux   **************************************************
    // *************************************************************************
    Result GetLux(uint16_t& lux);

    // *************************************************************************
    // ***   Public: GetColorTemperature   *************************************
    // *************************************************************************
    Result GetColorTemperature(uint16_t& color_temperature);

    // *************************************************************************
    // ***   Public: SetLimits   ***********************************************
    // *************************************************************************
    Result SetLimits(uint16_t low, uint16_t high);

    // *************************************************************************
    // ***   Public: SetInterrupt   ********************************************
    // *************************************************************************
    Result SetInterrupt(bool enable);

    // *************************************************************************
    // ***   Public: ClearInterrupt   ******************************************
    // *************************************************************************
    Result ClearInterrupt(void);

    // *************************************************************************
    // ***   Public: Read   ****************************************************
    // *************************************************************************
    Result Read(uint16_t addr, uint8_t* rx_buf_ptr, uint16_t size);

    // *************************************************************************
    // ***   Public: Write   ***************************************************
    // *************************************************************************
    Result Write(uint16_t addr, uint8_t* tx_buf_ptr, uint16_t size);

  private:

    enum RegisterType : uint8_t
    {
      REGISTER_ENABLE  = 0x00,
      REGISTER_ATIME   = 0x01, // Integration time
      REGISTER_WTIME   = 0x03, // Wait time(when ENABLE_WEN is set)
      REGISTER_AILTL   = 0x04, // Clear channel lower interrupt threshold (Lo)
      REGISTER_AILTH   = 0x05, // Clear channel lower interrupt threshold (Hi)
      REGISTER_AIHTL   = 0x06, // Clear channel upper interrupt threshold (Lo)
      REGISTER_AIHTH   = 0x07, // Clear channel upper interrupt threshold (Hi)
      REGISTER_PERS    = 0x0C, // Persistence register - basic SW filtering mechanism for interrupts
      REGISTER_CONFIG  = 0x0D,
      REGISTER_CONTROL = 0x0F, // Set the gain level for the sensor
      REGISTER_ID      = 0x12, // 0x44 = TCS34721/TCS34725, 0x4D = TCS34723/TCS34727
      REGISTER_STATUS  = 0x13,
      REGISTER_CDATAL  = 0x14, // Clear channel data (Lo)
      REGISTER_CDATAH  = 0x15, // Clear channel data (Hi)
      REGISTER_RDATAL  = 0x16, // Red channel data (Lo)
      REGISTER_RDATAH  = 0x17, // Red channel data (Hi)
      REGISTER_GDATAL  = 0x18, // Green channel data (Lo)
      REGISTER_GDATAH  = 0x19, // Green channel data (Hi)
      REGISTER_BDATAL  = 0x1A, // Blue channel data (Lo)
      REGISTER_BDATAH  = 0x1B  // Blue channel data (Hi)
    };

    enum EnableRegisterValueType : uint8_t
    {
      ENABLE_AIEN = 0x10, // RGBC Interrupt Enable
      ENABLE_WEN  = 0x08, // Wait Enable - Writing 1 activate the wait timer
      ENABLE_AEN  = 0x02, // RGBC Enable - Writing 1 activate the ADC, 0 disables it
      ENABLE_PON  = 0x01  // Power on - Writing 1 activates the internal oscillator, 0 disables it
    };

    enum StatusRegisterValueType : uint8_t
    {
      STATUS_AINT   = 0x10, // RGBC Clean channel interrupt
      STATUS_AVALID = 0x01  // Indicates that the RGBC channels have completed an integration cycle
    };

    enum ConfigRegisterValueType : uint8_t
    {
      CONFIG_WLONG  = 0x02 // Choose between short and long (12x) wait times via REGISTER_WTIME
    };

    enum PersRegisterValueType : uint8_t
    {
      PERS_NONE     = 0b0000, // Every RGBC cycle generates an interrupt
      PERS_1_CYCLE  = 0b0001, //  1 clear channel value outside of threshold range
      PERS_2_CYCLE  = 0b0010, //  2 clear channel consecutive values out of range
      PERS_3_CYCLE  = 0b0011, //  3 clean channel consecutive values out of range
      PERS_5_CYCLE  = 0b0100, //  5 clean channel consecutive values out of range
      PERS_10_CYCLE = 0b0101, // 10 clean channel consecutive values out of range
      PERS_15_CYCLE = 0b0110, // 15 clean channel consecutive values out of range
      PERS_20_CYCLE = 0b0111, // 20 clean channel consecutive values out of range
      PERS_25_CYCLE = 0b1000, // 25 clean channel consecutive values out of range
      PERS_30_CYCLE = 0b1001, // 30 clean channel consecutive values out of range
      PERS_35_CYCLE = 0b1010, // 35 clean channel consecutive values out of range
      PERS_40_CYCLE = 0b1011, // 40 clean channel consecutive values out of range
      PERS_45_CYCLE = 0b1100, // 45 clean channel consecutive values out of range
      PERS_50_CYCLE = 0b1101, // 50 clean channel consecutive values out of range
      PERS_55_CYCLE = 0b1110, // 55 clean channel consecutive values out of range
      PERS_60_CYCLE = 0b1111  // 60 clean channel consecutive values out of range
    };

    // Command bit
    static const uint8_t COMMAND_BIT = 0x80U;

    // Default chip address
    static const uint8_t I2C_ADDR = 0x29U;

    // Reference to the I2C handle
    IIic& iic;
    // I2C address
    uint8_t i2c_addr = I2C_ADDR;

    // Parameters
    bool inited = false;
    GainType gain = GAIN_1X;
    IntegrationTimeType integration_time = INTEGRATIONTIME_154MS;

    // Last values
    uint16_t red = 0U;
    uint16_t green = 0U;
    uint16_t blue = 0U;
    uint16_t clear = 0U;

    // *************************************************************************
    // ***   Write register value(8-bit)   *************************************
    // *************************************************************************
    Result Write8(uint8_t reg, uint8_t value);

    // *************************************************************************
    // ***   Read register value(8-bit unsigned)   *****************************
    // *************************************************************************
    Result Read8(uint8_t reg, uint8_t& value);

    // *************************************************************************
    // ***   Read register value(16-bit unsigned)   ****************************
    // *************************************************************************
    Result Read16(uint8_t reg, uint16_t& value, bool reverse = false);

    // *************************************************************************
    // ***   Private: Constructors and assign operator - prevent copying   *****
    // *************************************************************************
    Tcs34725();
    Tcs34725(const Tcs34725&);
    Tcs34725& operator=(const Tcs34725);
};

#endif
