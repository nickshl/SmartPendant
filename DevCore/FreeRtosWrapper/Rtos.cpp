//******************************************************************************
//  @file Rtos.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: FreeRTOS Wrapper, implementation
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
#include "Rtos.h"

// *****************************************************************************
// ***   TaskCreate   **********************************************************
// *****************************************************************************
Result Rtos::TaskCreate(TaskFunction& function, const char* task_name,
                        const uint16_t stack_depth, void* param_ptr,
                        uint8_t priority)
{
  Result result = Result::ERR_TASK_CREATE;
  // We need task handle to set Tag
  TaskHandle_t task_andle = nullptr;
  // Create task: function - TaskFunWrapper(), parameter - pointer "this"
  BaseType_t res = xTaskCreate(&function, task_name, stack_depth, param_ptr, priority, &task_andle);
  // Check result
  if(res == pdPASS)
  {
    // Set task function to Application Tag to use later
    vTaskSetApplicationTaskTag(task_andle, (TaskHookFunction_t)param_ptr);
    // Result ok
    result = Result::RESULT_OK;
  }

  return result;
}

// *****************************************************************************
// ***   IsInHandlerMode   *****************************************************
// *****************************************************************************
bool Rtos::IsInHandlerMode()
{
  return __get_IPSR() != 0;
}
