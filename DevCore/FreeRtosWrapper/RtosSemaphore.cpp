//******************************************************************************
//  @file RtosSemaphore.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: FreeRTOS Semaphore Wrapper Class, implementation
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
#include "RtosSemaphore.h"
#include "Rtos.h"

// *****************************************************************************
// ***   Constructor   *********************************************************
// *****************************************************************************
RtosSemaphore::RtosSemaphore()
{
  // Create semaphore
  semaphore = xSemaphoreCreateBinary();
  // Check error
  if(semaphore == nullptr)
  {
    // TODO: implement error handling
    Break();
  }
}

// *****************************************************************************
// ***   Destructor   **********************************************************
// *****************************************************************************
RtosSemaphore::~RtosSemaphore()
{
  if(semaphore == nullptr)
  {
    vSemaphoreDelete(semaphore);
  }
}

// *****************************************************************************
// ***   Take   ****************************************************************
// *****************************************************************************
Result RtosSemaphore::Take(TickType_t ticks_to_wait)
{
  Result result;
  // Variable for check result
  BaseType_t res;

  // Check handler mode
  if(Rtos::IsInHandlerMode())
  {
    BaseType_t task_woken;
    // Take semaphore from ISR
    res = xSemaphoreTakeFromISR(semaphore, &task_woken);
    // Switch context if needed
    portEND_SWITCHING_ISR((res == pdPASS) ? task_woken : pdFALSE);
  }
  else
  {
    // Take semaphore
    res = xSemaphoreTake(semaphore, ticks_to_wait);
  }

  // Find result
  if(res == pdTRUE)
  {
    result = Result::RESULT_OK;
  }
  else
  {
    result = Result::ERR_SEMAPHORE_TAKE;
  }

  // Return result
  return result;
}


// *****************************************************************************
// ***   Take   ****************************************************************
// *****************************************************************************
Result RtosSemaphore::Give()
{
  Result result;
  // Variable for check result
  BaseType_t res = xSemaphoreGive(semaphore);

  // Check handler mode
  if(Rtos::IsInHandlerMode())
  {
    BaseType_t task_woken;
    // Give semaphore from ISR
    res = xSemaphoreGiveFromISR(semaphore, &task_woken);
    // Switch context if needed
    portEND_SWITCHING_ISR((res == pdPASS) ? task_woken : pdFALSE);
  }
  else
  {
    // Give semaphore
    res = xSemaphoreGive(semaphore);
  }

  // Find result
  if(res == pdTRUE)
  {
    result = Result::RESULT_OK;
  }
  else
  {
    result = Result::ERR_SEMAPHORE_GIVE;
  }

  // Return result
  return result;
}
