//******************************************************************************
//  @file RtosRecursiveMutex.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: FreeRTOS Mutex Wrapper Class, implementation
//
//  @copyright Copyright (c) 2022, Devtronic & Nicolai Shlapunov
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
#include "RtosRecursiveMutex.h"
#include "Rtos.h"

#if (configUSE_RECURSIVE_MUTEXES == 1)

// *****************************************************************************
// ***   Constructor   **********************************************************
// *****************************************************************************
RtosRecursiveMutex::RtosRecursiveMutex()
{
  // Create semaphore
#if (configSUPPORT_STATIC_ALLOCATION  == 1)
  mutex = xSemaphoreCreateRecursiveMutexStatic(&mutex_buf);
#else
  mutex = xSemaphoreCreateRecursiveMutex();
#endif

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
RtosRecursiveMutex::~RtosRecursiveMutex()
{
  if(mutex == nullptr)
  {
    vSemaphoreDelete(mutex);
  }
}

// *****************************************************************************
// ***   Lock   ****************************************************************
// *****************************************************************************
Result RtosRecursiveMutex::Lock(TickType_t ticks_to_wait)
{
  Result result;
  // Variable for check result
  BaseType_t res = pdFALSE;

  // Check handler mode
  if(Rtos::IsInHandlerMode())
  {
    // Recursive Mutex can't be taken from ISR
    result = Result::ERR_MUTEX_LOCK;
  }
  else
  {
    // Take mutex
    res = xSemaphoreTakeRecursive(mutex, ticks_to_wait);
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
Result RtosRecursiveMutex::Release()
{
  Result result;
  // Variable for check result
  BaseType_t res = pdFALSE;

  // Check handler mode
  if(Rtos::IsInHandlerMode())
  {
    // Recursive Mutex can't be given from ISR
    result = Result::ERR_MUTEX_RELEASE;
  }
  else
  {
    // Give mutex
    res = xSemaphoreGiveRecursive(mutex);
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

#endif
