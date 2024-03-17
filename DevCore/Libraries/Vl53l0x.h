//******************************************************************************
//  @file Vl53l0x.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: STM TOF VL53L0X Driver driver, header
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

#ifndef Vl53l0x_h
#define Vl53l0x_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "IIic.h"

// *****************************************************************************
// ***   STM TOF VL53L0X Driver Class   ****************************************
// *****************************************************************************
class Vl53l0x
{
  public:
    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    explicit Vl53l0x(IIic& iic_ref) : iic(iic_ref) {};

    // *************************************************************************
    // ***   Public: Initialize   **********************************************
    // *************************************************************************
    Result Initialize();

    // *************************************************************************
    // ***   Public: Get distance in mm   **************************************
    // *************************************************************************
    Result GetDistanceMm(uint16_t& distance);

  private:

    // Register addresses from API vl53l0x_device.h (ordered as listed there)
    enum RegsAddr
    {
      SYSRANGE_START                              = 0x00,

      SYSTEM_THRESH_HIGH                          = 0x0C,
      SYSTEM_THRESH_LOW                           = 0x0E,

      SYSTEM_SEQUENCE_CONFIG                      = 0x01,
      SYSTEM_RANGE_CONFIG                         = 0x09,
      SYSTEM_INTERMEASUREMENT_PERIOD              = 0x04,

      SYSTEM_INTERRUPT_CONFIG_GPIO                = 0x0A,

      GPIO_HV_MUX_ACTIVE_HIGH                     = 0x84,

      SYSTEM_INTERRUPT_CLEAR                      = 0x0B,

      RESULT_INTERRUPT_STATUS                     = 0x13,
      RESULT_RANGE_STATUS                         = 0x14,

      RESULT_CORE_AMBIENT_WINDOW_EVENTS_RTN       = 0xBC,
      RESULT_CORE_RANGING_TOTAL_EVENTS_RTN        = 0xC0,
      RESULT_CORE_AMBIENT_WINDOW_EVENTS_REF       = 0xD0,
      RESULT_CORE_RANGING_TOTAL_EVENTS_REF        = 0xD4,
      RESULT_PEAK_SIGNAL_RATE_REF                 = 0xB6,

      ALGO_PART_TO_PART_RANGE_OFFSET_MM           = 0x28,

      I2C_SLAVE_DEVICE_ADDRESS                    = 0x8A,

      MSRC_CONFIG_CONTROL                         = 0x60,

      PRE_RANGE_CONFIG_MIN_SNR                    = 0x27,
      PRE_RANGE_CONFIG_VALID_PHASE_LOW            = 0x56,
      PRE_RANGE_CONFIG_VALID_PHASE_HIGH           = 0x57,
      PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT          = 0x64,

      FINAL_RANGE_CONFIG_MIN_SNR                  = 0x67,
      FINAL_RANGE_CONFIG_VALID_PHASE_LOW          = 0x47,
      FINAL_RANGE_CONFIG_VALID_PHASE_HIGH         = 0x48,
      FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT = 0x44,

      PRE_RANGE_CONFIG_SIGMA_THRESH_HI            = 0x61,
      PRE_RANGE_CONFIG_SIGMA_THRESH_LO            = 0x62,

      PRE_RANGE_CONFIG_VCSEL_PERIOD               = 0x50,
      PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI          = 0x51,
      PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO          = 0x52,

      SYSTEM_HISTOGRAM_BIN                        = 0x81,
      HISTOGRAM_CONFIG_INITIAL_PHASE_SELECT       = 0x33,
      HISTOGRAM_CONFIG_READOUT_CTRL               = 0x55,

      FINAL_RANGE_CONFIG_VCSEL_PERIOD             = 0x70,
      FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI        = 0x71,
      FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO        = 0x72,
      CROSSTALK_COMPENSATION_PEAK_RATE_MCPS       = 0x20,

      MSRC_CONFIG_TIMEOUT_MACROP                  = 0x46,

      SOFT_RESET_GO2_SOFT_RESET_N                 = 0xBF,
      IDENTIFICATION_MODEL_ID                     = 0xC0,
      IDENTIFICATION_REVISION_ID                  = 0xC2,

      OSC_CALIBRATE_VAL                           = 0xF8,

      GLOBAL_CONFIG_VCSEL_WIDTH                   = 0x32,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_0            = 0xB0,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_1            = 0xB1,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_2            = 0xB2,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_3            = 0xB3,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_4            = 0xB4,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_5            = 0xB5,

      GLOBAL_CONFIG_REF_EN_START_SELECT           = 0xB6,
      DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD         = 0x4E,
      DYNAMIC_SPAD_REF_EN_START_OFFSET            = 0x4F,
      POWER_MANAGEMENT_GO1_POWER_FORCE            = 0x80,

      VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV           = 0x89,

      ALGO_PHASECAL_LIM                           = 0x30,
      ALGO_PHASECAL_CONFIG_TIMEOUT                = 0x30,
    };

    enum SeqEn
    {
      SEQUENCE_ENABLE_FINAL_RANGE = 0x80,
      SEQUENCE_ENABLE_PRE_RANGE   = 0x40,
      SEQUENCE_ENABLE_TCC         = 0x10,
      SEQUENCE_ENABLE_DSS         = 0x08,
      SEQUENCE_ENABLE_MSRC        = 0x04,
    };

    struct SequenceStepTimeouts
    {
      uint16_t pre_range_vcsel_period_pclks;
      uint16_t final_range_vcsel_period_pclks;
      uint16_t msrc_dss_tcc_mclks;
      uint16_t pre_range_mclks;
      uint16_t final_range_mclks;
      uint32_t msrc_dss_tcc_us;
      uint16_t pre_range_us;
      uint16_t final_range_us;
   };

    // Default chip address
    static const uint8_t I2C_ADDR = 0x29U;

    // Reference to the I2C handle
    IIic& iic;
    // I2C address
    uint8_t i2c_addr = I2C_ADDR;

    uint8_t stop_variable = 0U; // read by init and used when starting measurement; is StopVariable field of VL53L0X_DevData_t structure in API

    uint32_t measurement_timing_budget_us = 0U;

    // *************************************************************************
    // ***   SetVcselPulsePeriod   *********************************************
    // *************************************************************************
    enum VcselPeriodType{VcselPeriodPreRange, VcselPeriodFinalRange};
    Result SetVcselPulsePeriod(VcselPeriodType type, uint8_t period_pclks);

    // *************************************************************************
    // ***   Set Signal Rate Limit   *******************************************
    // *************************************************************************
    Result SetSignalRateLimit_x100(uint32_t limit_mcps);

    // *************************************************************************
    // ***   Get Spad Info   ***************************************************
    // *************************************************************************
    Result GetSpadInfo(uint8_t& count, bool& type_is_aperture);

    // *************************************************************************
    // ***   Get Measurement Timing Budget   ***********************************
    // *************************************************************************
    Result GetMeasurementTimingBudget(uint32_t& budget);

    // *************************************************************************
    // ***   Set Measurement Timing Budget   ***********************************
    // *************************************************************************
    Result SetMeasurementTimingBudget(uint32_t budget_us);

    // *************************************************************************
    // ***   PerformSingleRefCalibration   *************************************
    // *************************************************************************
    Result PerformSingleRefCalibration(uint8_t vhv_init_byte);

    // *************************************************************************
    // ***   EncodeTimeout   ***************************************************
    // *************************************************************************
    uint16_t EncodeTimeout(uint16_t timeout_mclks);

    // *************************************************************************
    // ***   Get Measurement Timing Budget   ***********************************
    // *************************************************************************
    Result GetSequenceStepTimeouts(uint8_t enables, SequenceStepTimeouts& timeouts);

    // *************************************************************************
    // ***   TimeoutMclksToMicroseconds   **************************************
    // *************************************************************************
    uint32_t TimeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks);

    // *************************************************************************
    // ***   TimeoutMicrosecondsToMclks   **************************************
    // *************************************************************************
    uint32_t TimeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks);

    // *************************************************************************
    // ***   Write register value(8-bit)   *************************************
    // *************************************************************************
    Result Write8(uint8_t reg, uint8_t value);

    // *************************************************************************
    // ***   Read register value(8-bit unsigned)   *****************************
    // *************************************************************************
    Result Read8(uint8_t reg, uint8_t& value);

    // *************************************************************************
    // ***   Write register value(16-bit)   ************************************
    // *************************************************************************
    Result Write16(uint8_t reg, uint16_t value);

    // *************************************************************************
    // ***   Read register value(16-bit unsigned)   ****************************
    // *************************************************************************
    Result Read16(uint8_t reg, uint16_t& value);

    // *************************************************************************
    // ***   Write multiple registers   ****************************************
    // *************************************************************************
    Result WriteMulti(uint8_t reg, uint8_t* src, uint8_t count);

    // *************************************************************************
    // ***   Read multiple registers   *****************************************
    // *************************************************************************
    Result ReadMulti(uint8_t reg, uint8_t* dst, uint8_t count);

    // *************************************************************************
    // ***   Private: Constructors and assign operator - prevent copying   *****
    // *************************************************************************
    Vl53l0x();
    Vl53l0x(const Vl53l0x&);
    Vl53l0x& operator=(const Vl53l0x);
};

#endif
