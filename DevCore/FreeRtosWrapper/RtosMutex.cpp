//******************************************************************************
//  @file RtosMutex.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: FreeRTOS Mutex Wrapper Class, implementation
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
#include "RtosMutex.h"
#include "Rtos.h"

// *****************************************************************************
// ***   Constructor   *********************************************************
// *****************************************************************************
RtosMutex::RtosMutex()
{
  // Create semaphore
  mutex = xSemaphoreCreateMutex();
  // Check error
  if(mutex == nullptr)
  {
    // TODO: implement error handling
    Break();
  }
}

// *****************************************************************************
// ***   Destructor   **********************************************************
// *****************************************************************************
RtosMutex::~RtosMutex()
{
  if(mutex == nullptr)
  {
    vSemaphoreDelete(mutex);
  }
}

// *****************************************************************************
// ***   Lock   ****************************************************************
// *****************************************************************************
Result RtosMutex::Lock(TickType_t ticks_to_wait)
{
  Result result;
  // Variable for check result
  BaseType_t res = pdFALSE;

  // Check handler mode
  if(Rtos::IsInHandlerMode())
  {
    BaseType_t task_woken;
    // Take mutex from ISR
    res = xSemaphoreTakeFromISR(mutex, &task_woken);
    // Switch context if needed
    portEND_SWITCHING_ISR((res == pdPASS) ? task_woken : pdFALSE);
  }
  else
  {
    // Take mutex
    res = xSemaphoreTake(mutex, ticks_to_wait);
  }

  // Find result
  if(res == pdTRUE)
  {
    result = Result::RESULT_OK;
  }
  else
  {
    result = Result::ERR_MUTEX_LOCK;
  }

  // Return result
  return result;
}


// *****************************************************************************
// ***   Release   *************************************************************
// *****************************************************************************
Result RtosMutex::Release()
{
  Result result;
  // Variable for check result
  BaseType_t res = pdFALSE;

  // Check handler mode
  if(Rtos::IsInHandlerMode())
  {
    BaseType_t task_woken;
    // Give mutex from ISR
    res = xSemaphoreGiveFromISR(mutex, &task_woken);
    // Switch context if needed
    portEND_SWITCHING_ISR((res == pdPASS) ? task_woken : pdFALSE);
  }
  else
  {
    // Give mutex
    res = xSemaphoreGive(mutex);
  }

  // Find result
  if(res == pdTRUE)
  {
    result = Result::RESULT_OK;
  }
  else
  {
    result = Result::ERR_MUTEX_RELEASE;
  }
  // Return result
  return result;
}

// *****************************************************************************
// ***   IsTakenByCurrentTask   ************************************************
// *****************************************************************************
bool RtosMutex::IsTakenByCurrentTask(void)
{
  bool result = false;
  // Check status
  if(xSemaphoreGetMutexHolder(mutex) == xTaskGetCurrentTaskHandle())
  {
    result = true;
  }
  // Return result
  return result;
}
