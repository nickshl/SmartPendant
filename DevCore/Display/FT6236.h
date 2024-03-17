//******************************************************************************
//  @file FT6236.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: FT6236 Low Level Driver Class, header
//
//  @section LICENSE
//
//   Software License Agreement (BSD License)
//
//   Copyright (c) 2023, Devtronic & Nicolai Shlapunov
//   All rights reserved.
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are met:
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//   3. Neither the name of the Devtronic nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
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
//******************************************************************************

#ifndef FT6236_h
#define FT6236_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "ITouchscreen.h"
#include "IIic.h"
#include "IGpio.h"

// *****************************************************************************
// ***   FT6236 class. Implements work with FT6236 capacitive touchscreen   ****
// *****************************************************************************
class FT6236 : public ITouchscreen
{
  public:
    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    explicit FT6236(IIic& in_iic, ITouchscreen::Rotation orient = ROTATION_TOP, uint16_t w = 320u, uint16_t h = 480u) : iic(in_iic)
    {
      orientation = orient;
      width = w;
      height = h;
      // Update rotation to recalculate
      SetRotation(ROTATION_TOP);
    };

    // *************************************************************************
    // ***   Public: Init   ****************************************************
    // *************************************************************************
    // * Init function. Send init sequence to touchscreen controller.
    virtual Result Init(void);

    // *************************************************************************
    // ***   Public: IsTouch   *************************************************
    // *************************************************************************
    // * Check touched or not by T_IRQ pin. Return true if touched.
    virtual bool IsTouched(void);

    // *************************************************************************
    // ***   Public: GetRawXY   ************************************************
    // *************************************************************************
    // * Return raw X and Y coordinates. If touched - return true.
    virtual bool GetRawXY(int32_t& x, int32_t& y);

    // *************************************************************************
    // ***   Public: GetXY   ***************************************************
    // *************************************************************************
    // * Return recalculated using calibration constants X and Y coordinates.
    // * If touched - return true. Can be used for second calibration.
    virtual bool GetXY(int32_t& x, int32_t& y);

    // *************************************************************************
    // ***   Public: SetCalibrationConsts   ************************************
    // *************************************************************************
    virtual Result SetCalibrationConsts(int32_t nkx, int32_t nky, int32_t nbx, int32_t nby) {return Result::ERR_NOT_IMPLEMENTED;}

private:
    // Touch screen I2C address
    static constexpr uint8_t TOUCH_I2C_ADDR = 0x38;

    // Registers
    enum Register
    {
      REG_DEV_MODE         = 0x00, // Device mode, either WORKING or FACTORY
      REG_GEST_ID          = 0x01, // Describes the gesture of a valid touch
      REG_TD_STATUS        = 0x02, // The Touch Data status register
      REG_FACTORYMODE      = 0x40, // Factory mode
      REG_THRESHOLD        = 0x80, // Threshold for touch detection
      REG_CTRL             = 0x86, // Control Mode
      REG_TIMEENTERMONITOR = 0x87, // The time period of switching from Active mode to Monitor mode when there is no touching.
      REG_PERIODACTIVE     = 0x88, // Report rate in Active mode
      REG_PERIODMONITOR    = 0x89, // Report rate in Monitor mode
      REG_LIB_VER_H        = 0xA1, // High 8-bit of LIB Version info
      REG_LIB_VER_L        = 0xA2, // Low 8-bit of LIB Version info
      REG_CHIPER           = 0xA3, // Chip selecting
      REG_G_MODE           = 0xA4, // Interrupt mode
      REG_PWR_MODE         = 0xA5, // Current power mode which system is in
      REG_FIRMID           = 0xA6, // Firmware version
      REG_FOCALTECH_ID     = 0xA8, // FocalTech's panel ID
      REG_STATE            = 0xBC, // Current Operating mode
    };

    // CHIPER register options
    enum ChipEr
    {
      FT6206_CHIPER  = 0x06, // FT6206 ID
      FT6236_CHIPER  = 0x36, // FT6236 ID
      FT6236U_CHIPER = 0x64  // FT6236U ID
    };

    // FOCALTECH_ID register value
    static constexpr uint8_t FOCALTECH_ID = 0x11;

    // Default threshold
    static constexpr uint8_t DEFAULT_THRESHOLD = 40u;

//    #define FT6236_ADDR 0x38           // I2C address
//#define FT6236_G_FT5201ID 0xA8     // FocalTech's panel ID
//#define FT6236_REG_NUMTOUCHES 0x02 // Number of touch points
//
//#define FT6236_NUM_X 0x33 // Touch X position
//#define FT6236_NUM_Y 0x34 // Touch Y position
//
//#define FT6236_VENDID 0x11  // FocalTech's panel ID
//#define FT6206_CHIPID 0x06  // FT6206 ID
//#define FT6236_CHIPID 0x36  // FT6236 ID
//#define FT6236U_CHIPID 0x64 // FT6236U ID
//
//#define FT6236_DEFAULT_THRESHOLD 128 // Default threshold for touch detection

    // Handle to SPI used for touchscreen
    IIic& iic;

    // *************************************************************************
    // ***   Private: WriteReg8   **********************************************
    // *************************************************************************
    Result WriteReg8(uint8_t reg, uint8_t value);

    // *************************************************************************
    // ***   Private: ReadReg8   ***********************************************
    // *************************************************************************
    Result ReadReg8(uint8_t reg, uint8_t& value);

    uint8_t touch_cnt = 0u;
    uint16_t touch_x[2] = {0};
    uint16_t touch_y[2] = {0};
    uint16_t touch_id[2] = {0};
};

#endif