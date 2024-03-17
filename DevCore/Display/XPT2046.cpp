//******************************************************************************
//  @file XPT2046.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: XPT2046 Low Level Driver Class, implementation
//
//  @copyright Copyright (c) 2016, Devtronic & Nicolai Shlapunov
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
#include <XPT2046.h>

// *****************************************************************************
// ***   Public: Init touchscreen   ********************************************
// *****************************************************************************
Result XPT2046::Init(void)
{
  // Variable for original SPI clock
  ISpi::Mode spi_mode = ISpi::MODE_0;
  // Get original SPI clock
  Result mode_result = spi.GetMode(spi_mode);
  // If we successfully received speed
  if(mode_result.IsGood())
  {
    // Set MODE_0 for SPI
    mode_result = spi.SetMode(ISpi::MODE_0);
  }
  // Variable for original SPI clock
  uint32_t spi_clock = 0U;
  // Get original SPI clock
  Result speed_result = spi.GetSpeed(spi_clock);
  // If we successfully received speed
  if(speed_result.IsGood())
  {
    // Set SPI speed - max 2 MHz
    speed_result = spi.SetSpeed(2000000U);
  }

  // Pull down CS
  touch_cs.SetLow();
  // Send ON command
  SpiWrite(TON);
  // Send empty byte for skip answer
  SpiWrite(EMP);
  // Send empty byte for skip answer
  SpiWrite(EMP);
  // Pull up CS
  touch_cs.SetHigh();

  // Restore original SPI clock
  if(speed_result.IsGood())
  {
    spi.SetSpeed(spi_clock);
  }
  // Restore original SPI mode
  if(mode_result.IsGood())
  {
    spi.SetMode(spi_mode);
  }

  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Public: If touched - return true.   ***********************************
// *****************************************************************************
bool XPT2046::IsTouched(void)
{
  // Check T_IRQ input
  return touch_irq.IsLow();
}

// *****************************************************************************
// ***   Public: Get X and Y coordinates. If touched - return true.   **********
// *****************************************************************************
bool XPT2046::GetRawXY(int32_t& x, int32_t& y)
{
  // Return value
  bool ret = false;
  // If touch properly initialized and actual touch present
  if(IsTouched())
  {
    // Variable for original SPI clock
    uint32_t spi_clock = 0U;
    // Get original SPI clock
    Result speed_result = spi.GetSpeed(spi_clock);
    // If we successfully received speed
    if(speed_result.IsGood())
    {
      // Set SPI speed - max 2 MHz
      speed_result = spi.SetSpeed(2000000U);
    }
    // Variable for original SPI clock
    ISpi::Mode spi_mode = ISpi::MODE_0;
    // Get original SPI clock
    Result mode_result = spi.GetMode(spi_mode);
    // If we successfully received speed
    if(mode_result.IsGood())
    {
      // Set MODE_0 for SPI
      mode_result = spi.SetMode(ISpi::MODE_0);
    }

    // Pull down CS
    touch_cs.SetLow();
    // Request X coordinate
    SpiWrite(CHX);
    // Receive High byte for X
    x = SpiWriteRead(EMP) << 8;
    // Receive Low byte for X
    x |= SpiWriteRead(EMP);
    // Shift, because result have only 12 bits, 3 because answer started from
    // second rise edge
    x >>= 3;
    // Pull up CS
    touch_cs.SetHigh();

    // Pull down CS
    touch_cs.SetLow();
    // Request Y coordinate
    SpiWrite(CHY);
    // Receive High byte for Y
    y = SpiWriteRead(EMP) << 8;
    // Receive Low byte for Y
    y |= SpiWriteRead(EMP);
    // Shift, because result have only 12 bits, 3 because answer started from
    // second rise edge
    y >>= 3;
    // Pull up CS
    touch_cs.SetHigh();

    // Restore original SPI clock
    if(speed_result.IsGood())
    {
      spi.SetSpeed(spi_clock);
    }
    // Restore original SPI mode
    if(mode_result.IsGood())
    {
      spi.SetMode(spi_mode);
    }

    // Touch present
    ret = true;
  }
  // Return result
  return ret;
}

// *****************************************************************************
// ***   Public: Get X and Y coordinates. If touched - return true.   **********
// *****************************************************************************
bool XPT2046::GetXY(int32_t& x, int32_t& y)
{
  // Return value
  bool ret = GetRawXY(x, y);
  // If touch present
  if(ret)
  {
    // Calculate X
    x = ((x * COEF) / kx) + bx;
    // Calculate Y
    y = ((y * COEF) / ky) + by;
  }
  // Convert raw coordinates to coorinates including rotation
  if(ret)
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
  return ret;
}

// *****************************************************************************
// ***   Public: SetCalibrationConsts   ****************************************
// *****************************************************************************
Result XPT2046::SetCalibrationConsts(int32_t nkx, int32_t nky, int32_t nbx, int32_t nby)
{
  // Save calibration constants
  kx = nkx;
  ky = nky;
  bx = nbx;
  by = nby;
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Private: Write byte to SPI   ******************************************
// *****************************************************************************
inline void XPT2046::SpiWrite(uint8_t c)
{
  // Call HAL function for send byte by SPI
  spi.Write(&c, sizeof(c));
}

// *****************************************************************************
// ***   Private: Write and read byte to/from SPI   ****************************
// *****************************************************************************
inline uint8_t XPT2046::SpiWriteRead(uint8_t c)
{
  // Temporary variable for receive byte
  uint8_t rcv = 0U;
  // Call HAL function for send/receive byte by SPI
  spi.Transfer(&c, &rcv, sizeof(uint8_t));
  // Return received byte
  return rcv;
}
