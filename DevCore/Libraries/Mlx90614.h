//******************************************************************************
//  @file Mlx90614.h
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

#ifndef Mlx90614_h
#define Mlx90614_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "IIic.h"

// *****************************************************************************
// ***   Bosch BME280 library  *************************************************
// *****************************************************************************
class Mlx90614
{
  public:
    // *************************************************************************
    // ***   Object enum   *****************************************************
    // *************************************************************************
    enum ObjectType : uint8_t
    {
      OBJECT1 = 0U,
      OBJECT2,
      OBJECTS_CNT
    };

    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    Mlx90614(IIic& iic_ref) : iic(iic_ref) {};

    // *************************************************************************
    // ***   Initialize   ******************************************************
    // *************************************************************************
    Result Initialize(uint8_t addr = MLX90614_ADDRESS);

    // *************************************************************************
    // ***   TakeMeasurement   *************************************************
    // *************************************************************************
    Result TakeMeasurement();

    // *************************************************************************
    // ***   GetObjectTemperature_x100   ***************************************
    // *************************************************************************
    Result GetAmbientTemperature_x100(int32_t& temp_x100);

    // *************************************************************************
    // ***   GetObjectTemperature_x100   ***************************************
    // *************************************************************************
    Result GetObjectTemperature_x100(int32_t& temp_x100, ObjectType obj = OBJECT1);

  private:

    // ***   default I2C address **********************************************
    static const uint8_t MLX90614_ADDRESS = 0x5A;

    // ***   RAM Register addresses   *****************************************
    enum RamRegisters
    {
      RAM_RAWIR1 = 0x04,
      RAM_RAWIR2 = 0x05,
      RAM_TA     = 0x06,
      RAM_TOBJ1  = 0x07,
      RAM_TOBJ2  = 0x08,
    };

    // ***   EEPROM Register addresses   **************************************
    enum EepromRegisters
    {
      EEPROM_TOMAX   = 0x20,
      EEPROM_TOMIN   = 0x21,
      EEPROM_PWMCTRL = 0x22,
      EEPROM_TARANGE = 0x23,
      EEPROM_KE      = 0x24,
      EEPROM_CONFIG  = 0x25,
      EEPROM_ADDRESS = 0x2E,
      EEPROM_ID0     = 0x3C,
      EEPROM_ID1     = 0x3D,
      EEPROM_ID2     = 0x3E,
      EEPROM_ID3     = 0x3F,
      EEPROM_SLEEP   = 0xFF
    };

    // Reference to I2C interface
    IIic& iic;
    // I2C address
    uint8_t i2c_addr = MLX90614_ADDRESS;

    // Variables for store values
    int32_t temperature = 0;

    // *************************************************************************
    // ***   Read register value(16-bit unsigned)   ****************************
    // *************************************************************************
    Result ReadReg(uint8_t reg, uint16_t& value);

    // *************************************************************************
    // ***   Write register value(16-bit unsigned)   ***************************
    // *************************************************************************
    Result WriteReg(uint8_t reg, uint16_t value);

    // **************************************************************************
    // ***   Calculate CRC 8 for SMBus   ****************************************
    // **************************************************************************
    uint8_t Crc8(uint8_t data, uint8_t crc);
};

#endif
