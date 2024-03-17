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

#ifndef DwtCycleCounter_h
#define DwtCycleCounter_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"

#if defined(DWT_ENABLED)

// *****************************************************************************
// ***   DwtCycleCounter Class   ***********************************************
// *****************************************************************************
class DwtCycleCounter
{
  public:
    // *************************************************************************
    // ***   Init   ************************************************************
    // *************************************************************************
    static Result Init();

    // *************************************************************************
    // ***   GetClockCounter   *************************************************
    // *************************************************************************
    static inline uint32_t GetClockCounter() {return(DWT->CYCCNT);}

    // *************************************************************************
    // ***   DelayUs   *********************************************************
    // *************************************************************************
    static inline void DelayUs(volatile uint32_t delay_us)
    {
      uint32_t initial_ticks = DWT->CYCCNT;
      uint32_t ticks = (HAL_RCC_GetHCLKFreq() / 1000000u);
      delay_us *= ticks;
      while ((DWT->CYCCNT - initial_ticks) < delay_us - ticks);
    }
};

#endif

#endif
