//******************************************************************************
//  @file UiButton.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: UI Button Visual Object Class, implementation
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
#include "UiButton.h"

// *****************************************************************************
// *****************************************************************************
// ***   CheckBox   ************************************************************
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// ***   Constructor   *********************************************************
// *****************************************************************************
UiButton::UiButton(const char* str_in, int32_t x, int32_t y, int32_t w, int32_t h,
                   bool is_active)
{
  SetParams(str_in, x, y, w, h, is_active);
}

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void UiButton::SetParams(const char* str_in, int32_t x, int32_t y, int32_t w, int32_t h,
                         bool is_active)
{
  // Save string
  str = str_in;
  // VisObject variables
  x_start = x;
  y_start = y;
  x_end = x + w - 1;
  y_end = y + h - 1;
  width = w;
  height = h;
  active = is_active;
  // Set string params
  string.SetParams(str, x, y, color, Font_8x12::GetInstance());
  string.Move(x + (w - string.GetWidth()) / 2, y + (h - string.GetHeight()) / 2);
  string.SetAlignment(MultiLineString::CENTER);
  // Set shadow string params
  string_shadow.SetParams(str, string.GetStartX() + 1, string.GetStartY() + 1, COLOR_WHITE, Font_8x12::GetInstance());
  string_shadow.SetAlignment(MultiLineString::CENTER);
}

// *****************************************************************************
// ***   SetPosition   *********************************************************
// *****************************************************************************
void UiButton::SetPosition(int32_t x, int32_t y, int32_t w, int32_t h)
{
  // VisObject variables
  x_start = x;
  y_start = y;
  x_end = x + w - 1;
  y_end = y + h - 1;
  width = w;
  height = h;
  // Set string params
  string.SetParams(str, x, y, color, Font_8x12::GetInstance());
  string.Move(x + (w - string.GetWidth()) / 2, y + (h - string.GetHeight()) / 2);
  string.SetAlignment(MultiLineString::CENTER);
  // Set shadow string params
  string_shadow.SetParams(str, string.GetStartX() + 1, string.GetStartY() + 1, COLOR_WHITE, Font_8x12::GetInstance());
  string_shadow.SetAlignment(MultiLineString::CENTER);
}

// *****************************************************************************
// ***   SetString   ***********************************************************
// *****************************************************************************
void UiButton::SetString(const char* str_in)
{
  // If pointer have changed
  if(str_in != str)
  {
    // And string content is changed
    if(strcmp(str_in, str) != 0)
    {
      // Set string
      string.SetString(str_in);
      string.Move(x_start + (width - string.GetWidth()) / 2, y_start + (height - string.GetHeight()) / 2);
      // Set shadow string
      string_shadow.SetString(str_in);
      string_shadow.Move(string.GetStartX() + 1, string.GetStartY() + 1);
      // Invalidate area
      InvalidateObjArea();
    }
    // Save string pointer
    str = str_in;
  }
}

// *****************************************************************************
// ***   Public: SetColor   ****************************************************
// *****************************************************************************
void UiButton::SetColor(color_t c)
{
  if(c != color)
  {
    color = c;
    string.SetColor(c);
    // Invalidate area
    InvalidateObjArea();
  }
}

// *****************************************************************************
// ***   Public: SetFont   *****************************************************
// *****************************************************************************
void UiButton::SetFont(Font& font)
{
  string.SetFont(font);
  string.Move(x_start + (width - string.GetWidth()) / 2, y_start + (height - string.GetHeight()) / 2);
  // Set shadow string font and move it
  string_shadow.SetFont(font);
  string_shadow.Move(string.GetStartX() + 1, string.GetStartY() + 1);
}

// *****************************************************************************
// ***   Public: SetScale   ****************************************************
// *****************************************************************************
void UiButton::SetScale(uint8_t scale)
{
  string.SetScale(scale);
  string.Move(x_start + (width - string.GetWidth()) / 2, y_start + (height - string.GetHeight()) / 2);
  // Set shadow string font and move it
  string_shadow.SetScale(scale);
  string_shadow.Move(string.GetStartX() + 1, string.GetStartY() + 1);
}

// *****************************************************************************
// ***   Public: SetSpacing   **************************************************
// *****************************************************************************
void UiButton::SetSpacing(uint8_t spacing)
{
  string.SetSpacing(spacing);
  string.Move(x_start + (width - string.GetWidth()) / 2, y_start + (height - string.GetHeight()) / 2);
  // Set shadow string font and move it
  string_shadow.SetSpacing(spacing);
  string_shadow.Move(string.GetStartX() + 1, string.GetStartY() + 1);
}

// *****************************************************************************
// ***   Enable   **************************************************************
// *****************************************************************************
void UiButton::Enable()
{
  // Enable button only if it disabled
  if(IsActive() == false)
  {
    string.SetColor(color);
    SetActive(true);
    // Invalidate area
    InvalidateObjArea();
  }
}

// *****************************************************************************
// ***   Disable   *************************************************************
// *****************************************************************************
void UiButton::Disable()
{
  // Disable button only if it enabled
  if(IsActive())
  {
    SetActive(false);
    string.SetColor(COLOR_GREY);
    // Invalidate area
    InvalidateObjArea();
  }
}

// *****************************************************************************
// ***   SetPressed   **********************************************************
// *****************************************************************************
void UiButton::SetPressed(bool pressed)
{
  // Update status and invalidate area only if it really needed
  if(pressed != is_pressed)
  {
    // Set flag to draw pressed button
    is_pressed = pressed;
    // Invalidate area
    InvalidateObjArea();
  }
}

// *****************************************************************************
// ***   Set callback function   ***********************************************
// *****************************************************************************
void UiButton::SetCallback(AppTask* task, CallbackPtr func, void* param)
{
  callback_task = task;
  callback_func = func;
  callback_param = param;
}

// *****************************************************************************
// ***   Send callback function   **********************************************
// *****************************************************************************
void UiButton::SendCallback()
{
  // If AppTask pointer provided
  if(callback_task != nullptr)
  {
    // Call it to pass callback call to another task
    callback_task->Callback(callback_func, callback_param, this);
  }
  // If AppTask pointer is not provided, but callback function pointer provided
  else if(callback_func != nullptr)
  {
    // Call it in Display task(mutex may be needed inside callback!)
    callback_func(callback_param, this);
  }
  else
  {
    ; // Do nothing - MISRA rule
  }
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void UiButton::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
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
    if(end >= n) end = n - 1;
    if(end > 0)
    {
      // Color variable
      color_t c1 = (is_pressed || draw_pressed) ? COLOR_BLACK    : COLOR_WHITE;
      color_t c2 = (is_pressed || draw_pressed) ? COLOR_DARKGREY : COLOR_GREY;
      color_t c3 = (is_pressed || draw_pressed) ? COLOR_GREY     : COLOR_DARKGREY;
      color_t c4 = (is_pressed || draw_pressed) ? COLOR_WHITE    : COLOR_BLACK;
      color_t cb = (active) ? COLOR_GREY : COLOR_LIGHTGREY;

      if(line == y_start) cb = c1;
      else if(line == y_start + 1) cb = c2;
      else if(line == y_end - 1) cb = c3;
      else if(line == y_end) cb = c4;
      else cb = cb;

      // Fill buffer with background color
      for(int32_t i = start; i <= end; i++) buf[i] = cb;

      if((y_start < line) && (line < y_end))
      {
        if(x_start - start_x >= 0)
        {
          buf[start] = c1;
          buf[start + 1] = c2;
        }
        if(x_end - start_x < n)
        {
          buf[end - 1] = c3;
          buf[end] = c4;
        }
      }
    }
    // Draw shadow if button is disabled
    if(!active) string_shadow.DrawInBufW(buf, n, line, start_x);
    // Draw button text
    string.DrawInBufW(buf, n, line, start_x);
  }

//  box.DrawInBufW(buf, n, line, start_x);
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void UiButton::DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y)
{
  // TODO
  string.DrawInBufH(buf, n, row, start_y);
}

// *****************************************************************************
// ***   Action   **************************************************************
// *****************************************************************************
void UiButton::Action(VisObject::ActionType action, int32_t tx, int32_t ty, int32_t tpx, int32_t tpy)
{
  // Switch for process action
  switch(action)
  {
    // Touch action
    case VisObject::ACT_TOUCH:  // Fall thru
    case VisObject::ACT_MOVEIN:
      // Set pressed flag
      draw_pressed = true;
//      // Set string params
//      string.SetColor(COLOR_BLACK);
      // Invalidate area
      InvalidateObjArea();
      break;

    // Untouch action
    case VisObject::ACT_UNTOUCH:
      // Reset pressed flag before callback call since another task can set it again
      draw_pressed = false;
      // Set string params
      string.SetColor(color);
      // Send callback
      SendCallback();
      // Invalidate area
      InvalidateObjArea();
      break;

    case VisObject::ACT_MOVEOUT:
      // Reset pressed flag
      draw_pressed = false;
      // Set string params
      string.SetColor(color);
      // Invalidate area
      InvalidateObjArea();
      break;

    case VisObject::ACT_HOLD: // Intentional fall-trough
    case VisObject::ACT_MAX:
    default:
      break;
  }
}
