//******************************************************************************
//  @file RtosTick.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: FreeRTOS Tick Wrapper Class, implementation
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

#include "RtosTick.h"
#include "Rtos.h"

#include "portmacro.h"

// *****************************************************************************
// ***   GetTickCount   ********************************************************
// *****************************************************************************
uint32_t RtosTick::GetTickCount()
{
  uint32_t tick_count = 0U;

  // Check handler mode
  if(Rtos::IsInHandlerMode())
  {
    tick_count = xTaskGetTickCountFromISR();
  }
  else
  {
    tick_count = xTaskGetTickCount();
  }

  return tick_count;
}

// *****************************************************************************
// ***   GetTimeMs   ***********************************************************
// *****************************************************************************
uint32_t RtosTick::GetTimeMs()
{
  uint32_t time_ms = TicksToMs(GetTickCount());
  return time_ms;
}

// *****************************************************************************
// ***   DelayTicks   **********************************************************
// *****************************************************************************
void RtosTick::DelayTicks(uint32_t ticks)
{
  vTaskDelay(ticks);
}

// *****************************************************************************
// ***   DelayMs   *************************************************************
// *****************************************************************************
void RtosTick::DelayMs(uint32_t time_ms)
{
  vTaskDelay(MsToTicks(time_ms));
}

// *****************************************************************************
// ***   DelayUntilTicks   *****************************************************
// *****************************************************************************
void RtosTick::DelayUntilTicks(uint32_t& last_wake_ticks, uint32_t ticks)
{
  vTaskDelayUntil(&last_wake_ticks, ticks);
}

// *****************************************************************************
// ***   DelayUntilMs   ********************************************************
// *****************************************************************************
void RtosTick::DelayUntilMs(uint32_t& last_wake_ticks, uint32_t time_ms)
{
  vTaskDelayUntil(&last_wake_ticks, MsToTicks(time_ms));
}

// *****************************************************************************
// ***   MsToTicks   ***********************************************************
// *****************************************************************************
uint32_t RtosTick::MsToTicks(uint32_t time_ms)
{
  return(time_ms * portTICK_PERIOD_MS);
}

// *****************************************************************************
// ***   TicksToMs   ***********************************************************
// *****************************************************************************
uint32_t RtosTick::TicksToMs(uint32_t ticks)
{
  return(ticks / portTICK_PERIOD_MS);
}

