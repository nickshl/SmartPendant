//******************************************************************************
//  @file UiScroll.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: UI Scroll Visual Object Class, implementation
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
#include "UiScroll.h"

// *****************************************************************************
// ***   Constructor   *********************************************************
// *****************************************************************************
UiScroll::UiScroll(int32_t x, int32_t y, int32_t w, int32_t h, int32_t n, int32_t bar,
                   bool is_vertical, bool is_has_buttons, bool is_active)
{
  SetParams(x, y, w, h, n, bar, is_vertical, is_has_buttons, is_active);
}

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void UiScroll::SetParams(int32_t x, int32_t y, int32_t w, int32_t h, int32_t n, int32_t bar,
                         bool is_vertical, bool is_has_buttons, bool is_active)
{
  // General params
  x_start = x;
  y_start = y;
  x_end = x + w - 1;
  y_end = y + h - 1;
  width = w;
  height = h;
  active = is_active;
  // Private params
  cnt = 0;
  total_cnt = n;
  bar_cnt = bar;
  vertical = is_vertical;
  has_buttons = is_has_buttons;
  // Calculated data for drawing
  bar_shift = 1;
  total_len = (vertical ? height : width) - 2; // Total length for bar
  // Vertical with buttons
  if(has_buttons &&  vertical)
  {
    total_len -= width*2;
    bar_shift = width + 1;
  }
  // Horizontal with buttons
  if(has_buttons && !vertical)
  {
    total_len -= height*2;
    bar_shift = height + 1;
  }
  if(bar_cnt > total_cnt) bar_cnt = total_cnt; // Bar can't be greater total len
  bar_len   = (total_len * bar_cnt) / total_cnt;   // Bar length
  if(bar_len == 0) bar_len = 1; // Bar height can't be less than 1
  empty_len = total_len - bar_len; // Empty length available for bar moving
}

// *****************************************************************************
// ***   Set total number of lines   *******************************************
// *****************************************************************************
void UiScroll::SetTotal(int32_t val)
{
  // Update only if there change
  if(val != total_cnt)
  {
    // If new total value less than existing scroll position - update it first
    if(cnt > val) SetScrollPos(val);
    // Update bar first since we don't use semaphores
    if(bar_cnt > total_cnt) SetBar(val);
    // Update total
    total_cnt = val;
  }
}

// *****************************************************************************
// ***   Set bar width/height in scroll lines   ********************************
// *****************************************************************************
void UiScroll::SetBar(int32_t val)
{
  if((val != bar_cnt) && (val <= total_cnt))
  {
    bar_cnt = val;
    if(bar_cnt > total_cnt) bar_cnt = total_cnt; // Bar can't be greater total len
    bar_len   = (total_len * bar_cnt) / total_cnt;   // Bar length
    if(bar_len == 0) bar_len = 1; // Bar height can't be less than 1
    empty_len = total_len - bar_len; // Empty length available for bar moving
  }
}

// *****************************************************************************
// ***   Return End Y coordinate   *********************************************
// *****************************************************************************
void UiScroll::SetScrollPos(int32_t pos)
{
  if(pos != cnt)
  {
    cnt = pos;
    InvalidateObjArea();
  }
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void UiScroll::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
  // FIX ME: TEMPORARY COLOR !!!
  color_t color = COLOR_YELLOW;
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

    // Have sense draw only if end pointer in buffer
    if(x_end > 0)
    {
      // Draw border of scroll
      if((line == y_start) || (line == y_end))
      {
        for(int32_t i = start; i <= end; i++) buf[i] = color;
      }
      else if(    (has_buttons == true) && (vertical == true)
              && ((line == y_start + width) || (line == y_end - width + 1)) )
      {
        for(int32_t i = start; i <= end; i++) buf[i] = color;
      }
      else
      {
        if(x_start >= 0) buf[x_start] = color;
        if(x_end   <  n) buf[x_end]   = color;
        if(has_buttons && !vertical)
        {
          if(x_start + height >= 0) buf[x_start + height] = color;
          if(x_end   - height <  n) buf[x_end   - height] = color;
        }
      }
      // Find start of bar position
      int32_t bar_start = bar_shift;
      // Add bar additional shift
      bar_start += (empty_len * cnt) / (total_cnt - 1);
      if(vertical)
      {
        bar_start += y_start;
        // Draw bar line
        if((line >= bar_start) && (line < bar_start + bar_len))
        {
          for(int32_t i = start+1; i <= end-1; i++) buf[i] = COLOR_MAGENTA;
        }
      }
      else
      {
        if((line != y_start) && (line != y_end))
        {
          bar_start += x_start;
          // Prevent write in memory before buffer
          if(bar_start < 0) bar_start = 0;
          // Find start x position
          int32_t bar_end = bar_start + bar_len;
          // Prevent buffer overflow
          if(bar_end > n) bar_end = n;
          // Draw line
          for(int32_t i = bar_start; i < bar_end; i++) buf[i] = COLOR_MAGENTA;
        }
      }
    }
  }
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void UiScroll::DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y)
{
  // FIX ME: implement for Vertical Update Mode too 
}

// *****************************************************************************
// ***   Action   **************************************************************
// *****************************************************************************
void UiScroll::Action(VisObject::ActionType action, int32_t tx, int32_t ty, int32_t tpx, int32_t tpy)
{
  // Switch for process action
  switch(action)
  {
    // Touch action
    case VisObject::ACT_TOUCH:
      // Button touch processing
      if(has_buttons == true)
      {
        // If touched less than start pos plus width/height, we touch in square DEC btn
        if(   ( vertical && (ty < y_start + bar_shift))
           || (!vertical && (tx < x_start + bar_shift)) )
        {
          // Decrease counter
          cnt--;
          // If counter less than zero - correct it
          if(cnt < 0) cnt = 0;
        }
        // If touched greater than end pos minus width, we touch in square INC btn
        if(   ( vertical && (ty > y_end - bar_shift))
           || (!vertical && (tx > x_end - bar_shift)) )
        {
          // Increase counter
          cnt++;
          // If counter less than total count minus bar count - correct it
          if(cnt >= total_cnt) cnt = total_cnt-1;
        }
      }
      // Touch directly to bar
      if(vertical)
      {
        // Check touch in bar area
        if((ty > y_start + bar_shift) && (ty < y_end - bar_shift))
        {
          // Find press coordinates from bar start position
          int32_t press = (ty - y_start - bar_shift);
          // Calculate new count value
          cnt = (press * total_cnt) / total_len;
        }
      }
      else
      {
        // Check touch in bar area
        if((tx > x_start + bar_shift) && (tx < x_end - bar_shift))
        {
          // Find press coordinates from bar start position
          int32_t press = (tx - x_start - bar_shift);
          // Calculate new count value
          cnt = (press * total_cnt) / total_len;
        }
      }
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
