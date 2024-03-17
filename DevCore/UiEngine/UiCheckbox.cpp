//******************************************************************************
//  @file UiCheckbox.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: UI Checkbox Visual Object Class, implementation
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
#include "UiCheckbox.h"

// *****************************************************************************
// ***   Constructor   *********************************************************
// *****************************************************************************
UiCheckbox::UiCheckbox(int32_t x, int32_t y, bool is_checked, bool is_active)
{
  x_start = x;
  y_start = y;
  x_end = x + 13 - 1;
  y_end = y + 13 - 1;
  width = 13;
  height = 13;
  active = is_active;
  checked = is_checked;
}

// *****************************************************************************
// ***   Set callback function   ***********************************************
// *****************************************************************************
void UiCheckbox::SetCallback(AppTask& task, CallbackPtr func, void* param)
{
  callback_task = &task;
  callback_func = func;
  callback_param = param;
}

// *****************************************************************************
// ***   SetChecked   **********************************************************
// *****************************************************************************
void UiCheckbox::SetChecked(bool is_checked)
{
  // Set flag to draw pressed button
  checked = is_checked;
  // Invalidate area
  InvalidateObjArea();
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void UiCheckbox::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
  color_t color;
  // Draw only if needed
  if((line >= y_start) && (line <= y_end))
  {
    // Find start x position
    int32_t start = x_start - start_x;
    // Prevent write in memory before buffer
    if(start < 0) start = 0;
    // Find start x position
    int32_t end = x_end - start_x;
    // Prevent buffer overflow
    if(end > n) end = n;
    if(checked) color = COLOR_YELLOW;
    else        color = COLOR_MAGENTA;
    // Have sense draw only if end pointer in buffer
    if(x_end > 0)
    {
      // If fill or first/last row - must be solid
      if(true || line == y_start || line == y_end)
      {
        for(int32_t i = start; i <= end; i++) buf[i] = color;
      }
    }
  }
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void UiCheckbox::DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y)
{
  // FIX ME: implement for Vertical Update Mode too 
}

// *****************************************************************************
// ***   Action   **************************************************************
// *****************************************************************************
void UiCheckbox::Action(VisObject::ActionType action, int32_t tx, int32_t ty, int32_t tpx, int32_t tpy)
{
  // Switch for process action
  switch(action)
  {
    // Touch action
    case VisObject::ACT_TOUCH:
      // Change checked state
      checked = !checked;
      // Call callback after reset flag since another task can set it again
      if(callback_task != nullptr)
      {
        callback_task->Callback(callback_func, callback_param, this);
      }
      // Invalidate area
      InvalidateObjArea();
      break;

    // Untouch action 
    case VisObject::ACT_UNTOUCH:
      break;

    case VisObject::ACT_HOLD: // Intentional fall-trough
    case VisObject::ACT_MAX:
    default:
      break;
  }
}
