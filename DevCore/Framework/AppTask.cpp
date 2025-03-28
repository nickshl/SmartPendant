//******************************************************************************
//  @file AppTask.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: Application Task Base Class, implementation
//
//  @copyright Copyright (c) 2016, Devtronic & Nicolai Shlapunov
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
#include "AppTask.h"
#include "RtosMutex.h"

// *****************************************************************************
// ***   Static variables   ****************************************************
// *****************************************************************************
static RtosMutex startup_mutex;
static uint32_t startup_cnt = 0U;

// *****************************************************************************
// ***   Callback function   ***************************************************
// *****************************************************************************
Result AppTask::Callback(const CallbackPtr func_ptr, void* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // If callback call happens in the same task - no need to send it via queue,
  // call it right away. If timer period is zero and task queue doesn't exist, we
  // couldn't send callback via queue, so execute it in current task. Use mutex
  // inside callback in this case.
  if((this == GetCurrent()) || ((timer.GetTimerPeriod() == 0u) && (task_queue.GetQueueLen() == 0u)))
  {
    // If callback function pointer set
    if(func_ptr != nullptr)
    {
      // Call function by pointer
      result = func_ptr(obj_ptr, ptr);
    }
    else
    {
      // Otherwise call virtual function for callback process
      result = ProcessCallback(ptr);
    }
  }
  // To make a callback either func_ptr or ptr should be set
  else if((func_ptr != nullptr) || (ptr != nullptr))
  {
    // Create control timer message
    CtrlQueueMsg callback_msg;
    callback_msg.type = CTRL_CALLBACK_MSG;
    callback_msg.func_ptr = func_ptr;
    callback_msg.obj_ptr = obj_ptr;
    callback_msg.ptr = ptr;

    // Send message to the control queue
    result = SendControlMessage(callback_msg);
  }
  else
  {
    ; // Do nothing - MISRA rule
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Create task function   ************************************************
// *****************************************************************************
void AppTask::CreateTask()
{
  Result result = Result::RESULT_OK;

  // If interval timer period isn't zero or task queue present
  if((timer.GetTimerPeriod() != 0U) || (task_queue.GetQueueLen() != 0U))
  {
    // Set Control Queue name
    ctrl_queue.SetName(task_name, "Ctrl");
    // Create control queue
    result = ctrl_queue.Create();
  }
  // If task queue present
  if(task_queue.GetQueueLen() != 0U)
  {
    // Set Task Queue name
    task_queue.SetName(task_name, "Task");
    // Create task queue
    result |= task_queue.Create();
  }
  // If interval timer period isn't zero
  if(timer.GetTimerPeriod() != 0U)
  {
    // Create timer
    result |= timer.Create();
  }
  // Create task: function - TaskFunctionCallback(), parameter - pointer to "this"
  result |= Rtos::TaskCreate(TaskFunctionCallback, task_name, stack_size, this, task_priority);

  // Check result
  if(result.IsBad())
  {
    // TODO: implement error handling
    Break();
  }
}

// *****************************************************************************
// ***   SendTaskMessage function   ********************************************
// *****************************************************************************
Result AppTask::SendTaskMessage(const void* task_msg, bool is_task_priority, bool is_ctrl_priority)
{
  Result result = Result::RESULT_OK;

  // Send task message to front or back of task queue
  if(is_task_priority == true)
  {
    result = task_queue.SendToFront(task_msg);
  }
  else
  {
    result = task_queue.SendToBack(task_msg);
  }

  // If successful - send message to the control queue
  if(result.IsGood())
  {
    CtrlQueueMsg ctrl_msg;
    ctrl_msg.type = CTRL_TASK_QUEUE_MSG;
    // Only if both task and control priority set make sense to make send control
    // message as priority. Priority task and non-priority control is very useful
    // if task message can't be processed right away and have to be resend itself
    // to the front of task queue. To allow other control messages(timer, callback)
    // to be processed while task message loops task control message shouldn't be
    // sent as priority message
    result = SendControlMessage(ctrl_msg, is_task_priority && is_ctrl_priority);
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   IntLoop function   ****************************************************
// *****************************************************************************
Result AppTask::IntLoop()
{
  Result result = Result::RESULT_OK;

  while(result.IsGood())
  {
    // Buffer for control message
    CtrlQueueMsg ctrl_msg;
    // Read on the control queue
    result = ctrl_queue.Receive(&ctrl_msg, timer.GetTimerPeriod() * 2U);
    // If successful
    if(result.IsGood())
    {
      // Check message type
      switch(ctrl_msg.type)
      {
        case CTRL_TIMER_MSG:
          result = TimerExpired();
          break;

        case CTRL_CALLBACK_MSG:
          // If callback function pointer set
          if(ctrl_msg.func_ptr != nullptr)
          {
            // Call function by pointer
            result = ctrl_msg.func_ptr(ctrl_msg.obj_ptr, ctrl_msg.ptr);
          }
          else
          {
            // Otherwise call virtual function for callback process
            result = ProcessCallback(ctrl_msg.ptr);
          }
          break;

         case CTRL_TASK_QUEUE_MSG:
         {
           // Non blocking read from the task queue
           result = task_queue.Receive(task_msg_ptr, 0U);
           // If successful
           if(result.IsGood())
           {
             // Process it!
             result = ProcessMessage();
           }
           else
           {
             // TODO: implement error handling
             Break();
           }
           break;
         }

         default:
           result = Result::ERR_INVALID_ITEM;
           break;
      }
    }
    else
    {
      // TODO: implement error handling
      Break();
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   TaskFunctionCallback   ************************************************
// *****************************************************************************
void AppTask::TaskFunctionCallback(void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  if(ptr != nullptr)
  {
    // Set good result
    result = Result::RESULT_OK;
    // Get reference to the task object
    AppTask& app_task = *(static_cast<AppTask*>(ptr));

    // Increment counter before call Setup()
    ChangeCnt(true);
    // Call virtual Setup() function from AppTask class
    result = app_task.Setup();
    // Prevent other task to start if Setup() unsuccessful
    if(result.IsGood())
    {
      // Decrement counter after call Setup()
      ChangeCnt(false);
      // Pause for give other tasks run Setup()
      RtosTick::DelayTicks(1U);
      // Pause while other tasks run Setup() before executing any Loop()
      while(startup_cnt) RtosTick::DelayTicks(1U);

      // If no timer or queue - just call Loop() function
      if((app_task.timer.GetTimerPeriod() == 0U) && (app_task.task_queue.GetQueueLen() == 0U))
      {
        // Call virtual Loop() function from AppTask class
        while(app_task.Loop() == Result::RESULT_OK);
      }
      else
      {
        // Start task timer if needed
        if(app_task.timer.GetTimerPeriod() != 0U)
        {
          result = app_task.timer.Start();
        }
        // Check result
        if(result.IsGood())
        {
          // Call internal AppTask function
          result = app_task.IntLoop();
        }
        // Stop task timer if needed
        if(app_task.timer.GetTimerPeriod() != 0U)
        {
          result |= app_task.timer.Stop();
        }
      }
    }
  }

  // Check result
  if(result.IsBad())
  {
    // TODO: implement error handling
    Break();
  }

  // Delete task after exit
  Rtos::TaskDelete();
}

// *****************************************************************************
// ***   TimerCallback function   **********************************************
// *****************************************************************************
void AppTask::TimerCallback(void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  if(ptr != nullptr)
  {
    // Get reference to the task object
    AppTask& task = *((AppTask*)ptr);

    // Create control timer message
    CtrlQueueMsg timer_msg;
    timer_msg.type = CTRL_TIMER_MSG;

    // Send message to the control queue
    result = task.SendControlMessage(timer_msg, task.timer_priority);
  }

  // Check result
  if(result.IsBad())
  {
    // TODO: implement error handling
    Break();
  }
}

// *****************************************************************************
// ***   SendControlMessage function   *****************************************
// *****************************************************************************
Result AppTask::SendControlMessage(const CtrlQueueMsg& ctrl_msg, bool is_priority)
{
  Result result = Result::RESULT_OK;

  if(is_priority == true)
  {
    result = ctrl_queue.SendToFront(&ctrl_msg);
  }
  else
  {
    result = ctrl_queue.SendToBack(&ctrl_msg);
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Change counter   ******************************************************
// *****************************************************************************
void AppTask::ChangeCnt(bool is_up)
{
  // Take semaphore before change counter
  startup_mutex.Lock();
  // Check direction
  if(is_up == true)
  {
    // Increment counter
    startup_cnt++;
  }
  else
  {
    // Decrement counter
    startup_cnt--;
  }
  // Give semaphore after changes
  startup_mutex.Release();
}
