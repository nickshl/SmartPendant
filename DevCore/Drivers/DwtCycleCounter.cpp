//******************************************************************************
//  @file DwtCycleCounter.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: Cortex DWT Cycle counter driver, header
//
//  @copyright Copyright (c) 2023, Devtronic & Nicolai Shlapunov
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

#include "DwtCycleCounter.h"

#if defined(DWT_ENABLED)

// *****************************************************************************
// ***   Init   ****************************************************************
// *****************************************************************************
Result DwtCycleCounter::Init()
{
  Result result = Result::RESULT_OK;

  // Disable TRC
  CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
  // Enable TRC
  CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;
  // Unlock debug register
  DWT->LAR = 0xC5ACCE55;
  // Disable clock cycle counter
  DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
  // Enable clock cycle counter
  DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk; //0x00000001;
  // Reset the clock cycle counter value
  DWT->CYCCNT = 0u;
  // Check if counting is enabled. We do it not only to check it, but 
  // also to increase clock counter for next check.
  if(DWT->CTRL | DWT_CTRL_CYCCNTENA_Msk)
  {
    // Check if clock cycle counter has started
    if(DWT->CYCCNT == 0)
    {
        result = Result::ERR_CANNOT_EXECUTE; // Counter did not counted cycles by some reason
    }
  }
  else
  {
    result = Result::ERR_CANNOT_EXECUTE; // CYCCNTENA bit disabled - counting can't start
  }

  return result;
}

#endif
