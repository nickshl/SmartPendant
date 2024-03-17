//******************************************************************************
//  @file RtosTimer.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: FreeRTOS Timer Wrapper Class, header
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

#ifndef RtosTimer_h
#define RtosTimer_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "Rtos.h"
#include "timers.h"

// *****************************************************************************
// ***   RtosTimer   ***********************************************************
// *****************************************************************************
class RtosTimer
{
  public:
    // Definition of callback function
    typedef void (Callback)(void* ptr);

    // Types of timer
    enum TimerType
    {
       REPEATING,
       ONE_SHOT
    };

    // *************************************************************************
    // ***   RtosTimer   *******************************************************
    // *************************************************************************
    RtosTimer(uint32_t period_ms, TimerType type, Callback& clbk, void* clbk_param) :
              timer_period_ms(period_ms), timer_type(type),
              callback(&clbk), callback_param(clbk_param) {};

    // *************************************************************************
    // ***   ~RtosTimer   ******************************************************
    // *************************************************************************
    ~RtosTimer();

    // *************************************************************************
    // ***   Create   **********************************************************
    // *************************************************************************
    Result Create();

    // *************************************************************************
    // ***   IsActive   ********************************************************
    // *************************************************************************
    bool IsActive() const;

    // *************************************************************************
    // ***   Start   ***********************************************************
    // *************************************************************************
    Result Start(uint32_t timeout_ms = 0);

    // *************************************************************************
    // ***   Stop   ************************************************************
    // *************************************************************************
    Result Stop(uint32_t timeout_ms = 0);

    // *************************************************************************
    // ***   SetNewPeriod   ****************************************************
    // *************************************************************************
    Result UpdatePeriod(uint32_t new_period_ms, uint32_t timeout_ms = 0);

    // *************************************************************************
    // ***   StartWithNewPeriod   **********************************************
    // *************************************************************************
    Result StartWithNewPeriod(uint32_t new_period_ms, uint32_t timeout_ms = 0);

    // *************************************************************************
    // ***   Reset   ***********************************************************
    // *************************************************************************
    Result Reset(uint32_t timeout_ms = 0);

    // *************************************************************************
    // ***   GetTimerPeriod   **************************************************
    // *************************************************************************
    inline uint32_t GetTimerPeriod(void) const {return timer_period_ms;}

  private:
    // Timer handle
    TimerHandle_t timer = nullptr;

    // Timer period in ms
    uint32_t timer_period_ms = 0u;
    // Timer type
    TimerType timer_type;

    // Pointer to the callback function
    Callback* callback;
    // Pointer to the callback data
    void* callback_param = nullptr;

    // Timer callback wrapper function
    static void CallbackFunction(TimerHandle_t timer_handle);
};

#endif
