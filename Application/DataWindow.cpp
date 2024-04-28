//******************************************************************************
//  @file DataWindow.cpp
//  @author Nicolai Shlapunov
//
//  @details Application: User Application Class, implementation
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
#include "DataWindow.h"

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void DataWindow::SetParams(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t len, uint32_t ths)
{
  // Set data to -1 to update string later
  data = -1;

  // X and Y start coordinates of object
  x_start = x;
  y_start = y;
  // X and Y end coordinates of object
  x_end = x + w - 1;
  y_end = y + h - 1;
  // Width and Height of object
  width = w;
  height = h;

  // Save decimal
  before_decimal = len - ths;
  after_decimal = ths;
  // Set default multiplier
  decimal_multiplier = 1u;
  // Create multiplier
  for(int32_t i = 0; i < after_decimal; i++)
  {
    decimal_multiplier *= 10;
  }

  // Create format string
  if(ths)
  {
    snprintf(format_str_buf, NumberOf(format_str_buf), "%%%luld.%%0%lulu", before_decimal, after_decimal);
  }
  else
  {
    snprintf(format_str_buf, NumberOf(format_str_buf), "%%%luld", before_decimal);
  }

  // Set box parameters
  box.SetParams(0, 0, w, h, COLOR_BLACK, true);
  // Set data string parameters
  data_str.SetParams(data_str_buf, 0, 0, COLOR_WHITE, Font_8x12::GetInstance());
  // Data string length: sign, before decimal digits, '.' if we have decimal, after decimal digits
  uint8_t data_str_len = 1u + before_decimal + (after_decimal != 0u ? 1u : 0u) + after_decimal;
  data_str.Move((width - data_str.GetFontW() * data_str_len) / 2u, (h - data_str.GetHeight()) / 2, false);
  // Set units string parameters
  units_str.SetParams("", 0, 0, COLOR_WHITE, Font_8x12::GetInstance());

  // Invalidate area
  InvalidateObjArea();
}

// *****************************************************************************
// ***   SetDataFont   *********************************************************
// *****************************************************************************
void DataWindow::SetDataFont(Font& font, uint32_t scale)
{
  // Set data string parameters
  data_str.SetFont(font);
  // Set scale
  data_str.SetScale(scale);
  // Update data and units strings positions
  UpdateStringPositions();
  // Invalidate area
  InvalidateObjArea();
}

// *****************************************************************************
// ***   SetUnits   ************************************************************
// *****************************************************************************
void DataWindow::SetUnits(const char * str, Position pos, Font& font)
{
  // Save units string position
  units_str_pos = pos;
  // Set units string font
  units_str.SetFont(font);
  // Set units string
  units_str.SetString(str);
  // Update data and units strings positions
  UpdateStringPositions();
  // Invalidate area
  InvalidateObjArea();
}

// *****************************************************************************
// ***   SetBorder   ***********************************************************
// *****************************************************************************
void DataWindow::SetBorder(uint16_t w, color_t c)
{
  SetBorderWidth(w);
  SetBorderColor(c);
}

// *****************************************************************************
// ***   SetBorderWidth   ******************************************************
// *****************************************************************************
void DataWindow::SetBorderWidth(uint16_t w)
{
  // Update object only if there is a change - prevent invalidation of region
  // that doesn't need it
  if(w != box.GetBorderWidth())
  {
    // Set border width
    box.SetBorderWidth(w);
    // Invalidate area
    InvalidateObjArea();
  }
}

// *****************************************************************************
// ***   SetBorderColor   ******************************************************
// *****************************************************************************
void DataWindow::SetBorderColor(color_t cu, color_t cs)
{
  // Save colors
  border_color_unselected = cu;
  border_color_selected = cs;

  // Update object only if there is a change - prevent invalidation of region
  // that doesn't need it
  if(box.GetColor() != (selected ? border_color_selected : border_color_unselected))
  {
    // Set border color
    box.SetColor(selected ? border_color_selected : border_color_unselected);
    // Invalidate area
    InvalidateObjArea();
  }
}

// *****************************************************************************
// ***   SetNumber   ***********************************************************
// *****************************************************************************
bool DataWindow::SetNumber(int32_t n)
{
  bool chganged = false;
  // If data has changed
  if(n != data)
  {
    // Save new value
    data = n;
    // Update string
    if(after_decimal == 0)
    {
      data_str.SetString(data_str_buf, NumberOf(data_str_buf), format_str_buf, data);
    }
    else
    {
      data_str.SetString(data_str_buf, NumberOf(data_str_buf), format_str_buf, data / decimal_multiplier, abs(data % decimal_multiplier));
      // If n less than 0, but will give 0 in first part - we have to handle sign differently
      if((data < 0) && ((data / decimal_multiplier) == 0))
      {
        data_str_buf[before_decimal - 2] = '-';
      }
    }
    // Invalidate area
    InvalidateObjArea();
    // Data has changed - set flag
    chganged = true;
  }
  // Return result
  return chganged;
}

// *****************************************************************************
// ***   UpdateStringPositions   ***********************************************
// *****************************************************************************
void DataWindow::UpdateStringPositions(void)
{
  // Data string length: before decimal digits, '.' if we have decimal, after decimal digits
  int32_t data_str_len = before_decimal + (after_decimal ? 1u : 0u) + after_decimal;
  // Move strings
  if(units_str_pos == LEFT)
  {
// TODO:
//    data_str.Move((width - data_str.GetFontW() * data_str_len + units_str.GetWidth() + 2) / 2u, (height - data_str.GetHeight()) / 2, false);
//    units_str.Move(data_str.GetEndX() + 2, data_str.GetEndY() - units_str.GetFontH(), false);
  }
  else if(units_str_pos == RIGHT)
  {
    data_str.Move((width - data_str.GetFontW() * data_str.GetScale() * data_str_len - units_str.GetWidth() - 2) / 2, (height - data_str.GetHeight()) / 2, false);
    units_str.Move(data_str.GetEndX() + 2, data_str.GetEndY() - units_str.GetFontH(), false);
  }
  else if(units_str_pos == TOP)
  {
    units_str.Move((width - units_str.GetWidth()) / 2u, (height - data_str.GetHeight() - units_str.GetHeight()) / 2, false);
    data_str.Move((width - data_str.GetFontW() * data_str.GetScale() * data_str_len) / 2u, units_str.GetEndY(), false);
  }
  else if(units_str_pos == BOTTOM)
  {
    data_str.Move((width - data_str.GetFontW() * data_str.GetScale() * data_str_len) / 2u, (height - data_str.GetHeight()) / 2, false);
    units_str.Move((width - units_str.GetWidth()) / 2u, data_str.GetEndY(), false);
  }
  else if(units_str_pos == BOTTOM_RIGHT)
  {
    data_str.Move((width - data_str.GetFontW() * data_str.GetScale() * data_str_len) / 2u, (height - data_str.GetHeight()) / 2, false);
    units_str.Move(data_str.GetEndX() - units_str.GetWidth(), data_str.GetEndY(), false);
  }
  else //if(units_str_pos == NONE)
  {
    data_str.Move((width - data_str.GetFontW() * data_str.GetScale() * data_str_len) / 2u, (height - data_str.GetHeight()) / 2, false);
  }
  // Invalidate area
  InvalidateObjArea();
}

// *****************************************************************************
// ***   SetSeleced   **********************************************************
// *****************************************************************************
void DataWindow::SetSeleced(bool is_selected)
{
  // Ignore same state requests
  if(is_selected != selected)
  {
    // Save selected flag
    selected = is_selected;
    // Set border color
    box.SetColor(selected ? border_color_selected : border_color_unselected);
    // Invalidate area
    InvalidateObjArea();
  }
}


// *****************************************************************************
// ***   Set callback function   ***********************************************
// *****************************************************************************
void DataWindow::SetCallback(AppTask* task, CallbackPtr func, void* param)
{
  callback_task = task;
  callback_func = func;
  callback_param = param;
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void DataWindow::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
  box.DrawInBufW(buf, n, line - y_start, start_x - x_start);
  data_str.DrawInBufW(buf, n, line - y_start, start_x - x_start);
  if(units_str_pos != NONE) units_str.DrawInBufW(buf, n, line - y_start, start_x - x_start);
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void DataWindow::DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y)
{
  box.DrawInBufH(buf, n, row - x_start, start_y - y_start);
  data_str.DrawInBufH(buf, n, row - x_start, start_y - y_start);
  if(units_str_pos != NONE) units_str.DrawInBufH(buf, n, row - x_start, start_y - y_start);
}

// *****************************************************************************
// ***   Action   **************************************************************
// *****************************************************************************
void DataWindow::Action(VisObject::ActionType action, int32_t tx, int32_t ty, int32_t tpx, int32_t tpy)
{
  // Switch for process action
  switch(action)
  {
    // Touch action
    case VisObject::ACT_TOUCH:  // Fall thru
    case VisObject::ACT_MOVEIN:
      box.SetBackgroundColor(COLOR_GREY); // Set box color
      InvalidateObjArea(); // Invalidate area(box isn't in object list, so it will not invalidate itself)
      break;

    // Untouch action
    case VisObject::ACT_UNTOUCH:
      // Call callback
      if(callback_task != nullptr)
      {
        callback_task->Callback(callback_func, callback_param, this);
      }
      // Intentional fall-trough to change background color
    case VisObject::ACT_MOVEOUT:
      box.SetBackgroundColor(COLOR_BLACK); // Set original box color
      InvalidateObjArea(); // Invalidate area(box isn't in object list, so it will not invalidate itself)
      break;

    case VisObject::ACT_HOLD: // Intentional fall-trough
    case VisObject::ACT_MAX:
    default:
      break;
  }
}
