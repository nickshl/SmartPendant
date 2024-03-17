//******************************************************************************
//  @file UiTask.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: Sound Driver Class, implementation
//
//  @copyright Copyright (c) 2023, Devtronic & Nicolai Shlapunov
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
#include "UiTask.h"

#if defined(UITASK_ENABLED)

#include "Rtos.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
UiTask& UiTask::GetInstance(void)
{
  // This class is static and declared here
  static UiTask ui_task;
  // Return reference to object
  return ui_task;
}

// *****************************************************************************
// ***   Init Display Driver Task   ********************************************
// *****************************************************************************
void UiTask::InitTask()
{
  // Create task
  CreateTask();
}

// *****************************************************************************
// ***   Sound Driver Setup   **************************************************
// *****************************************************************************
Result UiTask::Setup()
{
  // Always ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Sound Driver Loop   ***************************************************
// *****************************************************************************
Result UiTask::Loop()
{
  // Always run
  return Result::RESULT_OK;
}

#endif
