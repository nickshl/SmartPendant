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

#ifndef RtosRecursiveMutex_h
#define RtosRecursiveMutex_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "Rtos.h"
#include "semphr.h"

#if (configUSE_RECURSIVE_MUTEXES == 1)

// *****************************************************************************
// ***   RtosRecursiveMutex   **************************************************
// *****************************************************************************
class RtosRecursiveMutex
{
  public:
    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    RtosRecursiveMutex();

    // *************************************************************************
    // ***   Destructor   ******************************************************
    // *************************************************************************
    ~RtosRecursiveMutex();

    // *************************************************************************
    // ***   Lock   ************************************************************
    // *************************************************************************
    Result Lock(TickType_t ticks_to_wait = portMAX_DELAY);

    // *************************************************************************
    // ***   Release   *********************************************************
    // *************************************************************************
    Result Release();

  private:
#if (configSUPPORT_STATIC_ALLOCATION  == 1)
    // Mutex buffer if static allocation is enabled
    StaticSemaphore_t mutex_buf;
#endif

    // Mutex handle
    SemaphoreHandle_t mutex;
};

#endif

#endif
