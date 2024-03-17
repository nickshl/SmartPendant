//******************************************************************************
//  @file Vl53l0x.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: STM TOF VL53L0X Driver driver, implementation
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
#include "Vl53l0x.h"

#include "string.h" // for memcpy()

// *****************************************************************************
// ***   Public: Initialize   **************************************************
// *****************************************************************************
Result Vl53l0x::Initialize()
{
  Result result = Result::RESULT_OK;
  iic.SetTxTimeout(10U);
  iic.SetRxTimeout(100U);

  uint8_t reg = 0U;

  // sensor uses 1V8 mode for I/O by default; switch to 2V8 mode if necessary
  result = Read8(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, reg);
  if(result.IsGood())
  {
    result = Write8(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, reg | 0x01); // set bit 0
  }

  // "Set I2C standard mode"
  if(result.IsGood()) result = Write8(0x88, 0x00);

  if(result.IsGood()) result = Write8(0x80, 0x01);
  if(result.IsGood()) result = Write8(0xFF, 0x01);
  if(result.IsGood()) result = Write8(0x00, 0x00);
  if(result.IsGood()) result = Read8(0x91, stop_variable);
  if(result.IsGood()) result = Write8(0x00, 0x01);
  if(result.IsGood()) result = Write8(0xFF, 0x00);
  if(result.IsGood()) result = Write8(0x80, 0x00);

  // disable SIGNAL_RATE_MSRC (bit 1) and SIGNAL_RATE_PRE_RANGE (bit 4) limit checks
  if(result.IsGood()) result = Read8(MSRC_CONFIG_CONTROL, reg);
  if(result.IsGood())
  {
    result = Write8(MSRC_CONFIG_CONTROL, reg | 0x12); // set bit 0
  }

  // Set final range signal rate limit to 0.25 MCPS (million counts per second)
  if(result.IsGood()) result = SetSignalRateLimit_x100(25U);

  if(result.IsGood()) result = Write8(SYSTEM_SEQUENCE_CONFIG, 0xFF);

  // VL53L0X_DataInit() end

  // VL53L0X_StaticInit() begin

  uint8_t spad_count = 0U;
  bool spad_type_is_aperture = false;
  if(result.IsGood()) result = GetSpadInfo(spad_count, spad_type_is_aperture);
  if(result.IsBad()) {return result;}

  // The SPAD map (RefGoodSpadMap) is read by VL53L0X_get_info_from_device() in
  // the API, but the same data seems to be more easily readable from
  // GLOBAL_CONFIG_SPAD_ENABLES_REF_0 through _6, so read it from there
  uint8_t ref_spad_map[6];
  if(result.IsGood()) result = ReadMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

  // -- VL53L0X_set_reference_spads() begin (assume NVM values are valid)

  if(result.IsGood()) result = Write8(0xFF, 0x01);
  if(result.IsGood()) result = Write8(DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
  if(result.IsGood()) result = Write8(DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
  if(result.IsGood()) result = Write8(0xFF, 0x00);
  if(result.IsGood()) result = Write8(GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);

  uint8_t first_spad_to_enable = spad_type_is_aperture ? 12 : 0; // 12 is the first aperture spad
  uint8_t spads_enabled = 0;

  for (uint8_t i = 0; i < 48; i++)
  {
    if (i < first_spad_to_enable || spads_enabled == spad_count)
    {
      // This bit is lower than the first one that should be enabled, or
      // (reference_spad_count) bits have already been enabled, so zero this bit
      ref_spad_map[i / 8] &= ~(1 << (i % 8));
    }
    else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x1)
    {
      spads_enabled++;
    }
  }

  if(result.IsGood()) result = WriteMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

  // -- VL53L0X_set_reference_spads() end

  // -- VL53L0X_load_tuning_settings() begin
  // DefaultTuningSettings from vl53l0x_tuning.h

  if(result.IsGood()) result = Write8(0xFF, 0x01);
  if(result.IsGood()) result = Write8(0x00, 0x00);

  if(result.IsGood()) result = Write8(0xFF, 0x00);
  if(result.IsGood()) result = Write8(0x09, 0x00);
  if(result.IsGood()) result = Write8(0x10, 0x00);
  if(result.IsGood()) result = Write8(0x11, 0x00);

  if(result.IsGood()) result = Write8(0x24, 0x01);
  if(result.IsGood()) result = Write8(0x25, 0xFF);
  if(result.IsGood()) result = Write8(0x75, 0x00);

  if(result.IsGood()) result = Write8(0xFF, 0x01);
  if(result.IsGood()) result = Write8(0x4E, 0x2C);
  if(result.IsGood()) result = Write8(0x48, 0x00);
  if(result.IsGood()) result = Write8(0x30, 0x20);

  if(result.IsGood()) result = Write8(0xFF, 0x00);
  if(result.IsGood()) result = Write8(0x30, 0x09);
  if(result.IsGood()) result = Write8(0x54, 0x00);
  if(result.IsGood()) result = Write8(0x31, 0x04);
  if(result.IsGood()) result = Write8(0x32, 0x03);
  if(result.IsGood()) result = Write8(0x40, 0x83);
  if(result.IsGood()) result = Write8(0x46, 0x25);
  if(result.IsGood()) result = Write8(0x60, 0x00);
  if(result.IsGood()) result = Write8(0x27, 0x00);
  if(result.IsGood()) result = Write8(0x50, 0x06);
  if(result.IsGood()) result = Write8(0x51, 0x00);
  if(result.IsGood()) result = Write8(0x52, 0x96);
  if(result.IsGood()) result = Write8(0x56, 0x08);
  if(result.IsGood()) result = Write8(0x57, 0x30);
  if(result.IsGood()) result = Write8(0x61, 0x00);
  if(result.IsGood()) result = Write8(0x62, 0x00);
  if(result.IsGood()) result = Write8(0x64, 0x00);
  if(result.IsGood()) result = Write8(0x65, 0x00);
  if(result.IsGood()) result = Write8(0x66, 0xA0);

  if(result.IsGood()) result = Write8(0xFF, 0x01);
  if(result.IsGood()) result = Write8(0x22, 0x32);
  if(result.IsGood()) result = Write8(0x47, 0x14);
  if(result.IsGood()) result = Write8(0x49, 0xFF);
  if(result.IsGood()) result = Write8(0x4A, 0x00);

  if(result.IsGood()) result = Write8(0xFF, 0x00);
  if(result.IsGood()) result = Write8(0x7A, 0x0A);
  if(result.IsGood()) result = Write8(0x7B, 0x00);
  if(result.IsGood()) result = Write8(0x78, 0x21);

  if(result.IsGood()) result = Write8(0xFF, 0x01);
  if(result.IsGood()) result = Write8(0x23, 0x34);
  if(result.IsGood()) result = Write8(0x42, 0x00);
  if(result.IsGood()) result = Write8(0x44, 0xFF);
  if(result.IsGood()) result = Write8(0x45, 0x26);
  if(result.IsGood()) result = Write8(0x46, 0x05);
  if(result.IsGood()) result = Write8(0x40, 0x40);
  if(result.IsGood()) result = Write8(0x0E, 0x06);
  if(result.IsGood()) result = Write8(0x20, 0x1A);
  if(result.IsGood()) result = Write8(0x43, 0x40);

  if(result.IsGood()) result = Write8(0xFF, 0x00);
  if(result.IsGood()) result = Write8(0x34, 0x03);
  if(result.IsGood()) result = Write8(0x35, 0x44);

  if(result.IsGood()) result = Write8(0xFF, 0x01);
  if(result.IsGood()) result = Write8(0x31, 0x04);
  if(result.IsGood()) result = Write8(0x4B, 0x09);
  if(result.IsGood()) result = Write8(0x4C, 0x05);
  if(result.IsGood()) result = Write8(0x4D, 0x04);

  if(result.IsGood()) result = Write8(0xFF, 0x00);
  if(result.IsGood()) result = Write8(0x44, 0x00);
  if(result.IsGood()) result = Write8(0x45, 0x20);
  if(result.IsGood()) result = Write8(0x47, 0x08);
  if(result.IsGood()) result = Write8(0x48, 0x28);
  if(result.IsGood()) result = Write8(0x67, 0x00);
  if(result.IsGood()) result = Write8(0x70, 0x04);
  if(result.IsGood()) result = Write8(0x71, 0x01);
  if(result.IsGood()) result = Write8(0x72, 0xFE);
  if(result.IsGood()) result = Write8(0x76, 0x00);
  if(result.IsGood()) result = Write8(0x77, 0x00);

  if(result.IsGood()) result = Write8(0xFF, 0x01);
  if(result.IsGood()) result = Write8(0x0D, 0x01);

  if(result.IsGood()) result = Write8(0xFF, 0x00);
  if(result.IsGood()) result = Write8(0x80, 0x01);
  if(result.IsGood()) result = Write8(0x01, 0xF8);

  if(result.IsGood()) result = Write8(0xFF, 0x01);
  if(result.IsGood()) result = Write8(0x8E, 0x01);
  if(result.IsGood()) result = Write8(0x00, 0x01);
  if(result.IsGood()) result = Write8(0xFF, 0x00);
  if(result.IsGood()) result = Write8(0x80, 0x00);

//  // Change some settings for long range mode
//  if(result.IsGood()) result = Write16(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 13); // 0.1
//  if(result.IsGood()) result = SetVcselPulsePeriod(VcselPeriodPreRange, 18);
//  if(result.IsGood()) result = SetVcselPulsePeriod(VcselPeriodFinalRange, 14);

  // -- VL53L0X_load_tuning_settings() end

  // "Set interrupt config to new sample ready"
  // -- VL53L0X_SetGpioConfig() begin

  if(result.IsGood()) result = Write8(SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
  if(result.IsGood()) result = Read8(GPIO_HV_MUX_ACTIVE_HIGH, reg);
  if(result.IsGood())
  {
    result = Write8(GPIO_HV_MUX_ACTIVE_HIGH, reg  & ~0x10); // active low
  }
  if(result.IsGood()) result = Write8(SYSTEM_INTERRUPT_CLEAR, 0x01);

  // -- VL53L0X_SetGpioConfig() end

  if(result.IsGood()) result = GetMeasurementTimingBudget(measurement_timing_budget_us);

  // "Disable MSRC and TCC by default"
  // MSRC = Minimum Signal Rate Check
  // TCC = Target CentreCheck
  // -- VL53L0X_SetSequenceStepEnable() begin

  if(result.IsGood()) result = Write8(SYSTEM_SEQUENCE_CONFIG, 0xE8);

  // -- VL53L0X_SetSequenceStepEnable() end

  // "Recalculate timing budget"
  if(result.IsGood()) result = SetMeasurementTimingBudget(measurement_timing_budget_us);

  // TODO: test!
  result = Result::RESULT_OK;

  // VL53L0X_StaticInit() end

  // VL53L0X_PerformRefCalibration() begin (VL53L0X_perform_ref_calibration())

  // -- VL53L0X_perform_vhv_calibration() begin

  if(result.IsGood()) result = Write8(SYSTEM_SEQUENCE_CONFIG, 0x01);
  if(result.IsGood()) result = PerformSingleRefCalibration(0x40);
  if(result.IsBad()) {return result;}

  // -- VL53L0X_perform_vhv_calibration() end

  // -- VL53L0X_perform_phase_calibration() begin

  if(result.IsGood()) result = Write8(SYSTEM_SEQUENCE_CONFIG, 0x02);
  if(result.IsGood()) result = PerformSingleRefCalibration(0x00);
  if(result.IsBad()) {return result;}

  // -- VL53L0X_perform_phase_calibration() end

  // "restore the previous Sequence Config"
  if(result.IsGood()) result = Write8(SYSTEM_SEQUENCE_CONFIG, 0xE8);

  // VL53L0X_PerformRefCalibration() end

  return result;
}

// *****************************************************************************
// ***   Public: Get distance in mm   ******************************************
// *****************************************************************************
Result Vl53l0x::GetDistanceMm(uint16_t& distance)
{
  Result result = Result::RESULT_OK;

  if(result.IsGood()) result = Write8(0x80, 0x01);
  if(result.IsGood()) result = Write8(0xFF, 0x01);
  if(result.IsGood()) result = Write8(0x00, 0x00);
  if(result.IsGood()) result = Write8(0x91, stop_variable);
  if(result.IsGood()) result = Write8(0x00, 0x01);
  if(result.IsGood()) result = Write8(0xFF, 0x00);
  if(result.IsGood()) result = Write8(0x80, 0x00);

  if(result.IsGood()) result = Write8(SYSRANGE_START, 0x01);

  // "Wait until start bit has been cleared"
  uint8_t val = 0U;
  if(result.IsGood())
  {
    while(((result = Read8(SYSRANGE_START, val)).IsGood()) && (val & 0x01))
    {
      // TODO: check timeout!
    }
  }

  if(result.IsGood())
  {
    while(((result = Read8(RESULT_INTERRUPT_STATUS, val)).IsGood()) && ((val & 0x07) == 0U))
    {
      // TODO: check timeout!
    }
  }

  // assumptions: Linearity Corrective Gain is 1000 (default);
  // fractional ranging is not enabled
  distance = 0U;
  if(result.IsGood()) result = Read16(RESULT_RANGE_STATUS + 10U, distance);

  if(result.IsGood()) result = Write8(SYSTEM_INTERRUPT_CLEAR, 0x01);

  return result;
}

// *****************************************************************************
// ***   SetVcselPulsePeriod   *************************************************
// *****************************************************************************
Result Vl53l0x::SetVcselPulsePeriod(VcselPeriodType type, uint8_t period_pclks)
{
  Result result = Result::RESULT_OK;

  // Set the VCSEL (vertical cavity surface emitting laser) pulse period for the
  // given period type (pre-range or final range) to the given value in PCLKs.
  // Longer periods seem to increase the potential range of the sensor.
  // Valid values are (even numbers only):
  //  pre:  12 to 18 (initialized default: 14)
  //  final: 8 to 14 (initialized default: 10)
  // based on VL53L0X_set_vcsel_pulse_period()

  uint8_t vcsel_period_reg = (((period_pclks) >> 1) - 1);

  uint8_t enables;
  SequenceStepTimeouts timeouts;

  if(result.IsGood()) result = Read8(SYSTEM_SEQUENCE_CONFIG, enables);
  if(result.IsGood()) result = GetSequenceStepTimeouts(enables, timeouts);

  // "Apply specific settings for the requested clock period"
  // "Re-calculate and apply timeouts, in macro periods"

  // "When the VCSEL period for the pre or final range is changed,
  // the corresponding timeout must be read from the device using
  // the current VCSEL period, then the new VCSEL period can be
  // applied. The timeout then must be written back to the device
  // using the new VCSEL period.
  //
  // For the MSRC timeout, the same applies - this timeout being
  // dependant on the pre-range vcsel period."

  if (type == VcselPeriodPreRange)
  {
    // "Set phase check limits"
    switch (period_pclks)
    {
      case 12:
        if(result.IsGood()) result = Write8(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x18);
        break;

      case 14:
        if(result.IsGood()) result = Write8(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x30);
        break;

      case 16:
        if(result.IsGood()) result = Write8(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x40);
        break;

      case 18:
        if(result.IsGood()) result = Write8(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x50);
        break;

      default:
        // invalid period
        return Result::ERR_BAD_PARAMETER;
    }
    if(result.IsGood()) result = Write8(PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);

    // apply new VCSEL period
    if(result.IsGood()) result = Write8(PRE_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);

    // update timeouts

    // set_sequence_step_timeout() begin
    // (SequenceStepId == VL53L0X_SEQUENCESTEP_PRE_RANGE)

    uint16_t new_pre_range_timeout_mclks = TimeoutMicrosecondsToMclks(timeouts.pre_range_us, period_pclks);

    if(result.IsGood()) result = Write16(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, EncodeTimeout(new_pre_range_timeout_mclks));

    // set_sequence_step_timeout() end

    // set_sequence_step_timeout() begin
    // (SequenceStepId == VL53L0X_SEQUENCESTEP_MSRC)

    uint16_t new_msrc_timeout_mclks = TimeoutMicrosecondsToMclks(timeouts.msrc_dss_tcc_us, period_pclks);

    if(result.IsGood()) result = Write8(MSRC_CONFIG_TIMEOUT_MACROP, (new_msrc_timeout_mclks > 256) ? 255 : (new_msrc_timeout_mclks - 1));

    // set_sequence_step_timeout() end
  }
  else if (type == VcselPeriodFinalRange)
  {
    switch (period_pclks)
    {
      case 8:
        if(result.IsGood()) result = Write8(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x10);
        if(result.IsGood()) result = Write8(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
        if(result.IsGood()) result = Write8(GLOBAL_CONFIG_VCSEL_WIDTH, 0x02);
        if(result.IsGood()) result = Write8(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x0C);
        if(result.IsGood()) result = Write8(0xFF, 0x01);
        if(result.IsGood()) result = Write8(ALGO_PHASECAL_LIM, 0x30);
        if(result.IsGood()) result = Write8(0xFF, 0x00);
        break;

      case 10:
        if(result.IsGood()) result = Write8(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x28);
        if(result.IsGood()) result = Write8(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
        if(result.IsGood()) result = Write8(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
        if(result.IsGood()) result = Write8(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x09);
        if(result.IsGood()) result = Write8(0xFF, 0x01);
        if(result.IsGood()) result = Write8(ALGO_PHASECAL_LIM, 0x20);
        if(result.IsGood()) result = Write8(0xFF, 0x00);
        break;

      case 12:
        if(result.IsGood()) result = Write8(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x38);
        if(result.IsGood()) result = Write8(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
        if(result.IsGood()) result = Write8(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
        if(result.IsGood()) result = Write8(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x08);
        if(result.IsGood()) result = Write8(0xFF, 0x01);
        if(result.IsGood()) result = Write8(ALGO_PHASECAL_LIM, 0x20);
        if(result.IsGood()) result = Write8(0xFF, 0x00);
        break;

      case 14:
        if(result.IsGood()) result = Write8(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x48);
        if(result.IsGood()) result = Write8(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
        if(result.IsGood()) result = Write8(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
        if(result.IsGood()) result = Write8(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x07);
        if(result.IsGood()) result = Write8(0xFF, 0x01);
        if(result.IsGood()) result = Write8(ALGO_PHASECAL_LIM, 0x20);
        if(result.IsGood()) result = Write8(0xFF, 0x00);
        break;

      default:
        // invalid period
        return Result::ERR_BAD_PARAMETER;
    }

    // apply new VCSEL period
    if(result.IsGood()) result = Write8(FINAL_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);

    // update timeouts

    // set_sequence_step_timeout() begin
    // (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

    // "For the final range timeout, the pre-range timeout
    //  must be added. To do this both final and pre-range
    //  timeouts must be expressed in macro periods MClks
    //  because they have different vcsel periods."

    uint16_t new_final_range_timeout_mclks = TimeoutMicrosecondsToMclks(timeouts.final_range_us, period_pclks);

    if(enables & SEQUENCE_ENABLE_PRE_RANGE)
    {
      new_final_range_timeout_mclks += timeouts.pre_range_mclks;
    }

    if(result.IsGood()) result = Write16(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, EncodeTimeout(new_final_range_timeout_mclks));

    // set_sequence_step_timeout end
  }
  else
  {
    // invalid type
    return Result::ERR_BAD_PARAMETER;
  }

  // "Finally, the timing budget must be re-applied"

  //if(result.IsGood()) result =
  SetMeasurementTimingBudget(measurement_timing_budget_us);

  // "Perform the phase calibration. This is needed after changing on vcsel period."
  // VL53L0X_perform_phase_calibration() begin

  uint8_t sequence_config = 0U;
  if(result.IsGood()) result = Read8(SYSTEM_SEQUENCE_CONFIG, sequence_config);
  if(result.IsGood()) result = Write8(SYSTEM_SEQUENCE_CONFIG, 0x02);
  if(result.IsGood()) result = PerformSingleRefCalibration(0x00);
  if(result.IsGood()) result = Write8(SYSTEM_SEQUENCE_CONFIG, sequence_config);

  // VL53L0X_perform_phase_calibration() end

  return result;
}

// *****************************************************************************
// ***   Set Signal Rate Limit   ***********************************************
// *****************************************************************************
Result Vl53l0x::SetSignalRateLimit_x100(uint32_t limit_mcps)
{
  Result result = Result::ERR_BAD_PARAMETER;

  // Set the return signal rate limit check value in units of MCPS (mega counts
  // per second). "This represents the amplitude of the signal reflected from the
  // target and detected by the device"; setting this limit presumably determines
  // the minimum measurement necessary for the sensor to report a valid reading.
  // Setting a lower limit increases the potential range of the sensor but also
  // seems to increase the likelihood of getting an inaccurate reading because of
  // unwanted reflections from objects other than the intended target.
  // Defaults to 0.25 MCPS as initialized by the ST API and this library.
  if (limit_mcps >= 0 || limit_mcps <= 51199U)
  {
    // Q9.7 fixed point format (9 integer bits, 7 fractional bits)
    result = Write16(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, (limit_mcps * 128U) / 100U);
  }

  return result;
}

// *****************************************************************************
// ***   Get Spad Info   *******************************************************
// *****************************************************************************
Result Vl53l0x::GetSpadInfo(uint8_t& count, bool& type_is_aperture)
{
  Result result = Result::RESULT_OK;

  uint8_t reg;

  result = Write8(0x80, 0x01);
  if(result.IsGood()) result = Write8(0xFF, 0x01);
  if(result.IsGood()) result = Write8(0x00, 0x00);

  if(result.IsGood()) result = Write8(0xFF, 0x06);
  if(result.IsGood()) result = Read8(0x83, reg);
  if(result.IsGood())
  {
    result = Write8(0x83, reg | 0x04); // active low
  }
  if(result.IsGood()) result = Write8(0xFF, 0x07);
  if(result.IsGood()) result = Write8(0x81, 0x01);

  if(result.IsGood()) result = Write8(0x80, 0x01);

  if(result.IsGood()) result = Write8(0x94, 0x6b);
  if(result.IsGood()) result = Write8(0x83, 0x00);
  if(result.IsGood())
  {
    while(((result = Read8(0x83, reg)).IsGood()) && (reg == 0x00))
    {
      // TODO: check timeout!
    }
  }
  if(result.IsGood()) result = Write8(0x83, 0x01);
  if(result.IsGood()) result = Read8(0x92, reg);

  count = reg & 0x7f;
  type_is_aperture = (reg >> 7) & 0x01;

  if(result.IsGood()) result = Write8(0x81, 0x00);
  if(result.IsGood()) result = Write8(0xFF, 0x06);
  if(result.IsGood()) result = Read8(0x83, reg);
  if(result.IsGood())
  {
    result = Write8(0x83, reg & ~0x04); // active low
  }
  if(result.IsGood()) result = Write8(0xFF, 0x01);
  if(result.IsGood()) result = Write8(0x00, 0x01);

  if(result.IsGood()) result = Write8(0xFF, 0x00);
  if(result.IsGood()) result = Write8(0x80, 0x00);

  return result;
}

// ******************************************************************************
// ***   Get Measurement Timing Budget   ****************************************
// ******************************************************************************
Result Vl53l0x::GetMeasurementTimingBudget(uint32_t& budget)
{
  Result result = Result::RESULT_OK;

  uint8_t enables;
  SequenceStepTimeouts timeouts;

  uint16_t const StartOverhead     = 1910; // note that this is different than the value in set_
  uint16_t const EndOverhead        = 960;
  uint16_t const MsrcOverhead       = 660;
  uint16_t const TccOverhead        = 590;
  uint16_t const DssOverhead        = 690;
  uint16_t const PreRangeOverhead   = 660;
  uint16_t const FinalRangeOverhead = 550;

  // "Start and end overhead times always present"
  uint32_t budget_us = StartOverhead + EndOverhead;

  result = Read8(SYSTEM_SEQUENCE_CONFIG, enables);
  if(result.IsGood()) result = GetSequenceStepTimeouts(enables, timeouts);

  if(enables & SEQUENCE_ENABLE_TCC)
  {
    budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
  }

  if(enables & SEQUENCE_ENABLE_DSS)
  {
    budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
  }
  else if(enables & SEQUENCE_ENABLE_MSRC)
  {
    budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
  }

  if(enables & SEQUENCE_ENABLE_PRE_RANGE)
  {
    budget_us += (timeouts.pre_range_us + PreRangeOverhead);
  }

  if(enables & SEQUENCE_ENABLE_FINAL_RANGE)
  {
    budget_us += (timeouts.final_range_us + FinalRangeOverhead);
  }

  measurement_timing_budget_us = budget_us; // store for internal reuse

  budget = budget_us;

  return result;
}

// *****************************************************************************
// ***   Set Measurement Timing Budget   ***************************************
// *****************************************************************************
Result Vl53l0x::SetMeasurementTimingBudget(uint32_t budget_us)
{
  Result result = Result::RESULT_OK;

  uint32_t used_budget_us;
  uint32_t final_range_timeout_us;
  uint16_t final_range_timeout_mclks;

  uint8_t enables;
  SequenceStepTimeouts timeouts;

  uint16_t const StartOverhead      = 1320; // note that this is different than the value in get_
  uint16_t const EndOverhead        = 960;
  uint16_t const MsrcOverhead       = 660;
  uint16_t const TccOverhead        = 590;
  uint16_t const DssOverhead        = 690;
  uint16_t const PreRangeOverhead   = 660;
  uint16_t const FinalRangeOverhead = 550;

  uint32_t const MinTimingBudget = 20000;

  if(budget_us < MinTimingBudget)
  {
    return Result::ERR_BAD_PARAMETER;
  }

  used_budget_us = StartOverhead + EndOverhead;

  result = Read8(SYSTEM_SEQUENCE_CONFIG, enables);
  if(result.IsGood()) result = GetSequenceStepTimeouts(enables, timeouts);

  if(enables & SEQUENCE_ENABLE_TCC)
  {
    used_budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
  }

  if(enables & SEQUENCE_ENABLE_DSS)
  {
    used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
  }
  else if(enables & SEQUENCE_ENABLE_MSRC)
  {
    used_budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
  }

  if(enables & SEQUENCE_ENABLE_PRE_RANGE)
  {
    used_budget_us += (timeouts.pre_range_us + PreRangeOverhead);
  }

  if(enables & SEQUENCE_ENABLE_FINAL_RANGE)
  {
    used_budget_us += FinalRangeOverhead;

    // "Note that the final range timeout is determined by the timing
    // budget and the sum of all other timeouts within the sequence.
    // If there is no room for the final range timeout, then an error
    // will be set. Otherwise the remaining time will be applied to
    // the final range."

    if(used_budget_us > budget_us)
    {
      // "Requested timeout too big."
      return Result::ERR_BAD_PARAMETER;
    }

    final_range_timeout_us = budget_us - used_budget_us;

    // set_sequence_step_timeout() begin
    // (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

    // "For the final range timeout, the pre-range timeout
    //  must be added. To do this both final and pre-range
    //  timeouts must be expressed in macro periods MClks
    //  because they have different vcsel periods."

    final_range_timeout_mclks = TimeoutMicrosecondsToMclks(final_range_timeout_us, timeouts.final_range_vcsel_period_pclks);

    if(enables & SEQUENCE_ENABLE_PRE_RANGE)
    {
      final_range_timeout_mclks += timeouts.pre_range_mclks;
    }

    uint16_t encoded_timeout = EncodeTimeout(final_range_timeout_mclks);
    if(result.IsGood()) result = Write16(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, encoded_timeout);

    // set_sequence_step_timeout() end

    measurement_timing_budget_us = budget_us; // store for internal reuse
  }

  return result;
}

// *****************************************************************************
// ***   PerformSingleRefCalibration   *****************************************
// *****************************************************************************
Result Vl53l0x::PerformSingleRefCalibration(uint8_t vhv_init_byte)
{
  Result result = Result::RESULT_OK;

  result = Write8(SYSRANGE_START, 0x01 | vhv_init_byte); // VL53L0X_REG_SYSRANGE_MODE_START_STOP

  uint8_t val = 0U;
  while(((result = Read8(RESULT_INTERRUPT_STATUS, val)).IsGood()) && ((val & 0x07) == 0x00))
  {
    // TODO: check timeout!
  }

  if(result.IsGood()) result = Write8(SYSTEM_INTERRUPT_CLEAR, 0x01);

  if(result.IsGood()) result = Write8(SYSRANGE_START, 0x00);

  return result;
}

// *****************************************************************************
// ***   EncodeTimeout   *******************************************************
// *****************************************************************************
uint16_t Vl53l0x::EncodeTimeout(uint16_t timeout_mclks)
{
  // format: "(LSByte * 2^MSByte) + 1"

  uint32_t ls_byte = 0;
  uint16_t ms_byte = 0;

  if (timeout_mclks > 0)
  {
    ls_byte = timeout_mclks - 1;

    while ((ls_byte & 0xFFFFFF00) > 0)
    {
      ls_byte >>= 1;
      ms_byte++;
    }

    return (ms_byte << 8) | (ls_byte & 0xFF);
  }
  else
  {
    return 0;
  }
}

// ******************************************************************************
// ***   Get Measurement Timing Budget   ****************************************
// ******************************************************************************
Result Vl53l0x::GetSequenceStepTimeouts(uint8_t enables, SequenceStepTimeouts& timeouts)
{
  Result result = Result::RESULT_OK;

  uint8_t val = 0U;
  uint16_t val16 = 0U;

  result = Read8(PRE_RANGE_CONFIG_VCSEL_PERIOD, val);
  timeouts.pre_range_vcsel_period_pclks = (val + 1) << 1;

  result = Read8(MSRC_CONFIG_TIMEOUT_MACROP, val);
  timeouts.msrc_dss_tcc_mclks = val + 1;
  timeouts.msrc_dss_tcc_us = TimeoutMclksToMicroseconds(timeouts.msrc_dss_tcc_mclks, timeouts.pre_range_vcsel_period_pclks);

  result = Read16(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, val16);
  timeouts.pre_range_mclks = (uint16_t)((val16 & 0x00FFU) << ((val16 >> 8U) & 0x00FFU)) + 1U;
  timeouts.pre_range_us = TimeoutMclksToMicroseconds(timeouts.pre_range_mclks, timeouts.pre_range_vcsel_period_pclks);

  result = Read8(FINAL_RANGE_CONFIG_VCSEL_PERIOD, val);
  timeouts.final_range_vcsel_period_pclks = (val + 1) << 1;

  result = Read16(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, val16);
  timeouts.final_range_mclks = (uint16_t)((val16 & 0x00FFU) << ((val16 >> 8U) & 0x00FFU)) + 1U;

  if(enables & SEQUENCE_ENABLE_PRE_RANGE)
  {
    timeouts.final_range_mclks -= timeouts.pre_range_mclks;
  }

  timeouts.final_range_us = TimeoutMclksToMicroseconds(timeouts.final_range_mclks, timeouts.final_range_vcsel_period_pclks);

  return result;
}

// *****************************************************************************
// ***   TimeoutMclksToMicroseconds   ******************************************
// *****************************************************************************
uint32_t Vl53l0x::TimeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks)
{
  uint32_t macro_period_ns = ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000);
  return ((timeout_period_mclks * macro_period_ns) + (macro_period_ns / 2)) / 1000;
}

// *****************************************************************************
// ***   TimeoutMicrosecondsToMclks   ******************************************
// *****************************************************************************
uint32_t Vl53l0x::TimeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks)
{
  uint32_t macro_period_ns = ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000);
  return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}

// *****************************************************************************
// ***   Write register value(8-bit)   *****************************************
// *****************************************************************************
Result Vl53l0x::Write8(uint8_t reg, uint8_t value)
{
  uint8_t buf[2];
  buf[0] = reg;
  buf[1] = value;
  return iic.Write(i2c_addr, buf, sizeof(buf));
}

// ******************************************************************************
// ***   Read register value(8-bit unsigned)   **********************************
// ******************************************************************************
Result Vl53l0x::Read8(uint8_t reg, uint8_t& value)
{
  // Transfer & return result
  return iic.Transfer(i2c_addr, &reg, sizeof(reg), (uint8_t*)&value, sizeof(value));
}

// ******************************************************************************
// ***   Write register value(16-bit)   *****************************************
// ******************************************************************************
Result Vl53l0x::Write16(uint8_t reg, uint16_t value)
{
  uint8_t buf[3];
  buf[0] = reg;
  buf[1] = (value >> 8) & 0x00FF;
  buf[2] = value & 0x00FF;
  return iic.Write(i2c_addr, buf, sizeof(buf));
}

// ******************************************************************************
// ***   Read register value(16-bit unsigned)   *********************************
// ******************************************************************************
Result Vl53l0x::Read16(uint8_t reg, uint16_t& value)
{
  Result result = Result::RESULT_OK;

  // Read data
  result = iic.Transfer(i2c_addr, &reg, sizeof(reg), (uint8_t*)&value, sizeof(value));
  // Change endian
  value = ((value >> 8U) & 0x00FF) | ((value << 8U) & 0xFF00);

  return result;
}

// ******************************************************************************
// ***   Write multiple registers   *********************************************
// ******************************************************************************
Result Vl53l0x::WriteMulti(uint8_t reg, uint8_t* src, uint8_t count)
{
  Result result = Result::ERR_BAD_PARAMETER;

  uint8_t buf[16];

  if(count < NumberOf(buf) - 1U)
  {
    buf[0] = reg;
    for(uint8_t i = 0U; i < count; i++)
    {
      buf[i + 1U] = src[i];
    }
    result = iic.Write(i2c_addr, buf, count + 1U);
  }

  return result;
}

// *************************************************************************
// ***   Read multiple registers   *****************************************
// *************************************************************************
Result Vl53l0x::ReadMulti(uint8_t reg, uint8_t* dst, uint8_t count)
{
  // Transfer & return result
  return iic.Transfer(i2c_addr, &reg, sizeof(reg), dst, count);
}
