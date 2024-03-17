//******************************************************************************
//  @file RtosTimer.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: FreeRTOS Timer Wrapper Class, implementation
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
#include "RtosTimer.h"
#include "Rtos.h"

#include "FreeRTOS.h"
#include "timers.h"

// *****************************************************************************
// ***   ~FreeRtosSoftwareTimer   **********************************************
// *****************************************************************************
RtosTimer::~RtosTimer()
{
  // If timer was created
  if(timer != nullptr)
  {
    // Clear callback handler
    callback = nullptr;
    // Delete timer
    (void) xTimerDelete(timer, 0U);
  }
}

// *****************************************************************************
// ***   Create   **************************************************************
// *****************************************************************************
Result RtosTimer::Create()
{
  Result result = Result::ERR_TIMER_CREATE;

  if(timer == nullptr)
  {
    // Create timer
    timer = xTimerCreate(nullptr, RtosTick::MsToTicks(timer_period_ms), (timer_type == REPEATING), this, CallbackFunction);
    // Check result
    if(timer != nullptr)
    {
      result = Result::RESULT_OK;
    }
  }

  return result;
}

// *****************************************************************************
// ***   IsActive   ************************************************************
// *****************************************************************************
bool RtosTimer::IsActive() const
{
  bool is_active = false;
  // Check timer handle
  if(timer != nullptr)
  {
    // Check timer state
    is_active = (xTimerIsTimerActive(timer) != pdFALSE);
  }
  // Return result
  return is_active;
}

// *****************************************************************************
// ***   Start   ***************************************************************
// *****************************************************************************
Result RtosTimer::Start(uint32_t timeout_ms)
{
  Result result = Result::ERR_TIMER_START;
  // Check timer handle
  if(timer != nullptr)
  {
    // Result of operation
    portBASE_TYPE res = pdFALSE;

    // Check handler mode
    if(Rtos::IsInHandlerMode())
    {
      BaseType_t task_woken;
      // Start timer from ISR
      res = xTimerStartFromISR(timer, &task_woken);
      // Switch context if needed
      portEND_SWITCHING_ISR((res == pdPASS) ? task_woken : pdFALSE);
    }
    else
    {
      // Start timer
      res = xTimerStart(timer, RtosTick::MsToTicks(timeout_ms));
    }
    // Check result
    if(res == pdPASS)
    {
      result = Result::RESULT_OK;
    }
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   Stop   ****************************************************************
// *****************************************************************************
Result RtosTimer::Stop(uint32_t timeout_ms)
{
  Result result = Result::ERR_TIMER_STOP;
  // Check timer handle
  if(timer != nullptr)
  {
    // Result of operation
    portBASE_TYPE res = pdFALSE;

    // Check handler mode
    if(Rtos::IsInHandlerMode())
    {
      BaseType_t task_woken;
      // Stop timer from ISR
      res = xTimerStopFromISR(timer, &task_woken);
      // Switch context if needed
      portEND_SWITCHING_ISR((res == pdPASS) ? task_woken : pdFALSE);
    }
    else
    {
      // Stop timer
      res = xTimerStop(timer, RtosTick::MsToTicks(timeout_ms));
    }
    // Check result
    if(res == pdPASS)
    {
      result = Result::RESULT_OK;
    }
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   UpdatePeriod   ********************************************************
// *****************************************************************************
Result RtosTimer::UpdatePeriod(uint32_t new_period_ms, uint32_t timeout_ms)
{
  Result result = Result::ERR_TIMER_UPDATE;
  // Check timer handle
  if(timer != nullptr)
  {
    // Get current timer state
    bool is_active = IsActive();
    // Start timer with new period
    result = StartWithNewPeriod(new_period_ms, timeout_ms);
    // StartWithNewPeriod() starts timer. So, if result is successful and timer
    // wasn't active, it should be stopped.
    if(result.IsGood() && (is_active == false))
    {
      // Stop timer
      result = Stop(timeout_ms);
    }
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   StartWithNewPeriod   **************************************************
// *****************************************************************************
Result RtosTimer::StartWithNewPeriod(uint32_t new_period_ms, uint32_t timeout_ms)
{
  Result result = Result::ERR_TIMER_UPDATE;
  // Check timer handle
  if(timer != nullptr)
  {
    // Update period
    timer_period_ms = new_period_ms;
    // Result of operation
    portBASE_TYPE res = pdFALSE;

    // Check handler mode
    if(Rtos::IsInHandlerMode())
    {
      BaseType_t task_woken;
      // Restart timer with new period from ISR
      res = xTimerChangePeriodFromISR(timer, RtosTick::MsToTicks(new_period_ms), &task_woken);
      // Switch context if needed
      portEND_SWITCHING_ISR((res == pdPASS) ? task_woken : pdFALSE);
    }
    else
    {
      // The FreeRTOS xTimerChangePeriod() function start timer
      res = xTimerChangePeriod(timer, RtosTick::MsToTicks(new_period_ms), RtosTick::MsToTicks(timeout_ms));
    }
    // Check result
    if(res == pdPASS)
    {
      result = Result::RESULT_OK;
    }
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   Reset   ***************************************************************
// *****************************************************************************
Result RtosTimer::Reset(uint32_t timeout_ms)
{
  Result result = Result::ERR_TIMER_STOP;
  // Check timer handle
  if(timer != nullptr)
  {
    // Result of operation
    portBASE_TYPE res = pdFALSE;

    // Check handler mode
    if(Rtos::IsInHandlerMode())
    {
      BaseType_t task_woken;
      // Reset timer from ISR
      res = xTimerResetFromISR(timer, &task_woken);
      // Switch context if needed
      portEND_SWITCHING_ISR((res == pdPASS) ? task_woken : pdFALSE);
    }
    else
    {
      // Reset timer
      res = xTimerReset(timer, RtosTick::MsToTicks(timeout_ms));
    }
    // Check result
    if(res == pdPASS)
    {
      result = Result::RESULT_OK;
    }
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   CallbackFunction   ****************************************************
// *****************************************************************************
void RtosTimer::CallbackFunction(TimerHandle_t timer_handle)
{
  // Check timer handle
  if(timer_handle != nullptr)
  {
    // Timer ID set to "this" pointer in Create()
    RtosTimer* this_ptr = static_cast<RtosTimer*>(pvTimerGetTimerID(timer_handle));

    if((this_ptr != nullptr) && (this_ptr->callback != nullptr))
    {
      // Execute callback
      this_ptr->callback(this_ptr->callback_param);
    }
  }
}


