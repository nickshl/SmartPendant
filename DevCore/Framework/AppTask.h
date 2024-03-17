//******************************************************************************
//  @file AppTask.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: Application Task Base Class, header
//
//  @section LICENSE
//
//   Software License Agreement (Modified BSD License)
//
//   Copyright (c) 2016, Devtronic & Nicolai Shlapunov
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

#ifndef AppTask_h
#define AppTask_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"

typedef Result (*CallbackPtr)(void* obj_ptr, void* ptr);

// *****************************************************************************
// * AppTask class. This class is wrapper for call C++ function from class. ****
// *****************************************************************************
class AppTask
{
  public:
    // *************************************************************************
    // ***   Init Task   *******************************************************
    // *************************************************************************
    virtual void InitTask(void) {CreateTask();}

    // *************************************************************************
    // ***   Callback   ********************************************************
    // *************************************************************************
    virtual Result Callback(CallbackPtr func_ptr, void* obj_ptr, void* ptr);

    // *************************************************************************
    // ***   GetCurrent   ******************************************************
    // *************************************************************************
    static inline AppTask* GetCurrent(void) {return reinterpret_cast<AppTask*>(Rtos::GetCurrentTaskParam());}

    // *************************************************************************
    // ***   StartTimer function   *********************************************
    // *************************************************************************
    void StartTimer() {timer.Start();}

    // *************************************************************************
    // ***   StopTimer function   **********************************************
    // *************************************************************************
    void StopTimer() {timer.Stop();}

  protected:
    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    AppTask(uint16_t stk_size, uint8_t task_prio, const char name[],
            uint16_t queue_len = 0U, uint16_t queue_msg_size = 0U,
            void* task_msg_p = nullptr, uint32_t task_interval_ms = 0U,
            bool tmr_priority = false) :
      ctrl_queue((queue_len + 2U), sizeof(CtrlQueueMsg)),
      task_queue(queue_len, queue_msg_size), task_msg_ptr(task_msg_p),
      timer(task_interval_ms, RtosTimer::REPEATING, TimerCallback, (void*)this),
      timer_priority(tmr_priority), stack_size(stk_size),
      task_priority(task_prio), task_name(name) {};

    // *************************************************************************
    // ***   Virtual destructor - prevent warning   ****************************
    // *************************************************************************
    virtual ~AppTask() {};

    // *************************************************************************
    // ***   Create task function   ********************************************
    // *************************************************************************
    // * This function creates new task in FreeRTOS, provide pointer to function
    // * and pointer to class as parameter. When TaskFunctionCallback() called
    // * from FreeRTOS, it use pointer to class from parameter to call virtual
    // * functions.
    void CreateTask();

    // *************************************************************************
    // ***   Setup function   **************************************************
    // *************************************************************************
    // * * virtual function - some tasks may not have Setup() actions
    virtual Result Setup() {return Result::RESULT_OK;}

    // *************************************************************************
    // ***   IntervalTimerExpired function   ***********************************
    // *************************************************************************
    // * Empty virtual function - some tasks may not have TimerExpired() actions
    virtual Result TimerExpired() {return Result::RESULT_OK;}

    // *************************************************************************
    // ***   ProcessMessage function   *****************************************
    // *************************************************************************
    // * Empty virtual function - some tasks may not have ProcessMessage() actions
    virtual Result ProcessMessage() {return Result::RESULT_OK;}

    // *************************************************************************
    // ***   ProcessCallback function   ****************************************
    // *************************************************************************
    // * Empty virtual function - some tasks may not have ProcessCallback() actions
    virtual Result ProcessCallback(const void* ptr) {return Result::RESULT_OK;}

    // *************************************************************************
    // ***   Loop function   ***************************************************
    // *************************************************************************
    // * Empty virtual function - some tasks may not have Loop() actions
    virtual Result Loop() {return Result::RESULT_OK;}

    // *************************************************************************
    // ***   SendTaskMessage function   ****************************************
    // *************************************************************************
    Result SendTaskMessage(const void* task_msg, bool is_task_priority = false, bool is_ctrl_priority = false);

  private:
    // Task control queue message types
    enum CtrlQueueMsgType
    {
       CTRL_TIMER_MSG,
       CTRL_CALLBACK_MSG,
       CTRL_TASK_QUEUE_MSG
    };
    // Task control queue message struct
    struct CtrlQueueMsg
    {
      CtrlQueueMsgType type;
      CallbackPtr func_ptr;
      void* obj_ptr;
      void* ptr;
    };
    // Task control queue
    RtosQueue ctrl_queue;

    // Task queue
    RtosQueue task_queue;
    // Pointer to receive message buffer
    void* task_msg_ptr;

    // Timer object
    RtosTimer timer;
    // Flag indicates that timer control message have to be priority
    bool timer_priority = false;

    // Task stack size
    uint16_t stack_size;
    // Task priority
    uint8_t task_priority;
    // Pointer to the task name
    const char* task_name;

    // *************************************************************************
    // ***   IntLoop function   ************************************************
    // *************************************************************************
    Result IntLoop();

    // *************************************************************************
    // ***   TaskFunctionCallback   ********************************************
    // *************************************************************************
    static void TaskFunctionCallback(void* ptr);

    // *************************************************************************
    // ***   IntervalTimerCallback function   **********************************
    // *************************************************************************
    static void TimerCallback(void* ptr);

    // *************************************************************************
    // ***   SendControlMessage function   *************************************
    // *************************************************************************
    Result SendControlMessage(const CtrlQueueMsg& ctrl_msg, bool is_priority = false);

    // *************************************************************************
    // ***   Change counter   **************************************************
    // *************************************************************************
    static void ChangeCnt(bool is_up);

    // *************************************************************************
    // ***   Private constructor and assign operator - prevent copying   *******
    // *************************************************************************
    AppTask();
    AppTask(const AppTask&);
    AppTask& operator=(const AppTask&);
};

#endif
