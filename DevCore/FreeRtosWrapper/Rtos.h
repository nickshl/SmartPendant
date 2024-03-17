//******************************************************************************
//  @file Rtos.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: FreeRTOS Wrapper, header
//
//  @section LICENSE
//
//   Software License Agreement (Modified BSD License)
//
//   Copyright (c) 2018, Devtronic & Nicolai Shlapunov
//   All rights reserved.
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//   3. Neither the name of the Devtronic nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//   4. Redistribution and use of this software other than as permitted under
//      this license is void and will automatically terminate your rights under
//      this license.
//
//   THIS SOFTWARE IS PROVIDED BY DEVTRONIC ''AS IS'' AND ANY EXPRESS OR IMPLIED
//   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL DEVTRONIC BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
//   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  @section SUPPORT
//
//   Devtronic invests time and resources providing this open source code,
//   please support Devtronic and open-source hardware/software by
//   donations and/or purchasing products from Devtronic.
//
//******************************************************************************

#ifndef Rtos_h
#define Rtos_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "FreeRTOS.h"
#include "task.h"

#include "DevCfg.h"
#include "RtosTick.h"
#include "RtosTimer.h"
#include "RtosQueue.h"
#include "RtosMutex.h"
#include "RtosSemaphore.h"

// *****************************************************************************
// ***   Rtos   ****************************************************************
// *****************************************************************************
class Rtos
{
  public:
    // Definition of callback function
    typedef void (TaskFunction)(void* ptr);

    // *************************************************************************
    // ***   TaskCreate   ******************************************************
    // *************************************************************************
    static Result TaskCreate(TaskFunction& function, const char* task_name,
                             const uint16_t stack_depth, void* param_ptr,
                             uint8_t priority);

    // *************************************************************************
    // ***   TaskDelete   ******************************************************
    // *************************************************************************
    static inline void TaskDelete(TaskHandle_t task = nullptr) {vTaskDelete(task);}

    // *************************************************************************
    // ***   GetCurrentTaskParam   *********************************************
    // *************************************************************************
    static inline void* GetCurrentTaskParam() {return (void*)xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());}

    // *************************************************************************
    // ***   IsInHandlerMode   *************************************************
    // *************************************************************************
    static bool IsInHandlerMode();

    // *************************************************************************
    // ***   IsSchedulerNotRunning   *******************************************
    // *************************************************************************
    static inline bool IsSchedulerNotRunning() {return (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED);}

    // *************************************************************************
    // ***   IsSchedulerRunning   **********************************************
    // *************************************************************************
    static inline bool IsSchedulerRunning() {return (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING);}

    // *************************************************************************
    // ***   IsSchedulerSuspended   ********************************************
    // *************************************************************************
    static inline bool IsSchedulerSuspended() {return (xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED);}

    // *************************************************************************
    // ***   SuspendScheduler   ************************************************
    // *************************************************************************
    static inline void SuspendScheduler() {vTaskSuspendAll();}

    // *************************************************************************
    // ***   ResumeScheduler   *************************************************
    // *************************************************************************
    static inline void ResumeScheduler() {(void)xTaskResumeAll();}

    // *************************************************************************
    // ***   EnterCriticalSection   ********************************************
    // *************************************************************************
    static inline void EnterCriticalSection() {taskENTER_CRITICAL();}

    // *************************************************************************
    // ***   ExitCriticalSection   *********************************************
    // *************************************************************************
    static inline void ExitCriticalSection() {taskEXIT_CRITICAL();}

    // *************************************************************************
    // ***   DisableInterrupts   ***********************************************
    // *************************************************************************
    static inline void DisableInterrupts() {taskDISABLE_INTERRUPTS();}

    // *************************************************************************
    // ***   EnableInterrupts   ************************************************
    // *************************************************************************
    static inline void EnableInterrupts() {taskENABLE_INTERRUPTS();}

 private:
    // None
};

#endif
