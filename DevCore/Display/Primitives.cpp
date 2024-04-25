//******************************************************************************
//  @file Primitives.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: Primitives Visual Object Classes(Box, Line, Circle), implementation
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
#include "Primitives.h"

#include <cstdlib> // for abs()

// *****************************************************************************
// *****************************************************************************
// ***   Box   *****************************************************************
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// ***   Constructor   *********************************************************
// *****************************************************************************
Box::Box(int32_t x, int32_t y, int32_t w, int32_t h, color_t c, bool is_fill)
{
  SetParams(x, y, w, h, c, is_fill);
}

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void Box::SetParams(int32_t x, int32_t y, int32_t w, int32_t h, color_t c, bool is_fill)
{
  // Lock object for changes
  LockVisObject();
  // Invalidate area for old position/size
  InvalidateObjArea();
  // Do changes
  color = c;
  bg_color = c;
  x_start = x;
  y_start = y;
  x_end = x + w - 1;
  y_end = y + h - 1;
  width = w;
  height = h;
  rotation = 0;
  fill = is_fill;
  border_width = is_fill ? 0 : 1; // Set border 1 pixel if border isn't fill
  // Invalidate area for new position/size
  InvalidateObjArea();
  // Unlock object after changes
  UnlockVisObject();
}

// *****************************************************************************
// ***   SetBorderWidth   ******************************************************
// *****************************************************************************
void Box::SetBorderWidth(int32_t width)
{
  // Lock object for changes
  LockVisObject();
  // Set new border width
  border_width = width;
  // Invalidate area
  InvalidateObjArea();
  // Unlock object after changes
  UnlockVisObject();
}

// *****************************************************************************
// ***   SetColor   ************************************************************
// *****************************************************************************
void Box::SetColor(color_t c)
{
  // Lock object for changes
  LockVisObject();
  // Do changes
  color = c;
  // Set background color the same if border width is zero
  if(border_width == 0) bg_color = c;
  // Invalidate area
  InvalidateObjArea();
  // Unlock object after changes
  UnlockVisObject();
}

// *****************************************************************************
// ***   SetBackgroundColor   **************************************************
// *****************************************************************************
void Box::SetBackgroundColor(color_t bgc)
{
  // Lock object for changes
  LockVisObject();
  // Do changes
  bg_color = bgc;
  // Invalidate area
  InvalidateObjArea();
  // Unlock object after changes
  UnlockVisObject();
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void Box::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
  // Draw only if needed
  if((line >= y_start) && (line <= y_end))
  {
    // Find start x position
    int32_t start = x_start - start_x;
    // Find start x position
    int32_t end = x_end - start_x;
    // Have sense draw only if end pointer in buffer
    if(end > 0)
    {
      // If fill or first/last line - must be solid
      if(fill)
      {
        // Prevent write in memory before buffer
        if(start < 0) start = 0;
        // Prevent buffer overflow
        if(end >= n) end = n - 1;
        // Fill the line
        for(int32_t i = start; i <= end; i++) buf[i] = bg_color;
      }
      // Fill top and bottom border lines (if border exist)
      if((line < y_start + border_width) || (line > y_end - border_width))
      {
        // Prevent write in memory before buffer
        if(start < 0) start = 0;
        // Prevent buffer overflow
        if(end >= n) end = n - 1;
        // Fill the line
        for(int32_t i = start; i <= end; i++) buf[i] = color;
      }
      else
      {
        // Fill left and right borders
        for(int32_t i = 0; i < border_width; i++)
        {
          int32_t front_idx = start + i;
          int32_t back_idx = (x_end - start_x) - i;
          if((front_idx >= 0) && (front_idx < n)) buf[front_idx] = color;
          if((back_idx  >= 0) && (back_idx  < n)) buf[back_idx]  = color;
        }
      }
    }
  }
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void Box::DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y)
{
  // Draw only if needed
  if((row >= x_start) && (row <= x_end))
  {
    // Find start x position
    int32_t start = y_start - start_y;
    // Prevent write in memory before buffer
    if(start < 0) start = 0;
    // Find start x position
    int32_t end = y_end - start_y;
    // Prevent buffer overflow
    if(end >= n) end = n - 1;
    // Have sense draw only if end pointer in buffer
    if(end > 0)
    {
      // If fill or first/last row - must be solid
      if(fill || row == x_start || row == x_end)
      {
        for(int32_t i = start; i <= end; i++) buf[i] = color;
      }
      else
      {
        if(y_start - start_y >= 0) buf[start] = color;
        if(y_end   - start_y <  n) buf[end]   = color;
      }
    }
  }
}

// *****************************************************************************
// *****************************************************************************
// ***   Line   ****************************************************************
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// ***   Constructor   *********************************************************
// *****************************************************************************
Line::Line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_t c)
{
  SetParams(x1, y1, x2, y2, c);
}

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void Line::SetParams(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_t c)
{
  // Lock object for changes
  LockVisObject();
  // Invalidate area for old position/size
  InvalidateObjArea();
  // Do changes
  color = c;
  x_start = x1;
  y_start = y1;
  x_end = x2;
  y_end = y2;
  width  = (x1 < x2) ? (x2 - x1) : (x1 - x2) + 1; // Width is one more than coordinates difference
  height = (y1 < y2) ? (y2 - y1) : (y1 - y2) + 1; // Height is one more than coordinates difference
  rotation = 0;
  // Invalidate area for new position/size
  InvalidateObjArea();
  // Unlock object after changes
  UnlockVisObject();
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void Line::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
  // Draw only if needed
  if(((line >= y_start) && (line <= y_end)) || ((line >= y_end) && (line <= y_start)))
  {
    const int32_t deltaX = abs(x_end - x_start);
    const int32_t deltaY = abs(y_end - y_start);
    const int32_t signX = x_start < x_end ? 1 : -1;
    const int32_t signY = y_start < y_end ? 1 : -1;

    int32_t error = deltaX - deltaY;

    int32_t x = x_start - start_x;
    int32_t y = y_start;

    int32_t end_x = x_end - start_x;
    while((x != end_x || y != y_end) && (y != line))
    {
      const int32_t error2 = error * 2;
      if(error2 > -deltaY) 
      {
        error -= deltaY;
        x += signX;
      }
      if(error2 < deltaX) 
      {
        error += deltaX;
        y += signY;
      }
    }

    // If we found current line
    if(y == line)
    {
      // Go and draw line
      do
      {
        if((x >= 0) && (x < n)) buf[x] = color;
        const int32_t error2 = error * 2;
        if(error2 > -deltaY) 
        {
          error -= deltaY;
          x += signX;
        }
        if(error2 < deltaX) 
        {
          break;
        }
      }
      while((x != end_x) || (y != y_end));
    }
  }
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void Line::DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y)
{
  // Draw only if needed
  if((row >= x_start) && (row <= x_end))
  {
    const int32_t deltaX = abs(y_end - y_start);
    const int32_t deltaY = abs(x_end - x_start);
    const int32_t signX = y_start < y_end ? 1 : -1;
    const int32_t signY = x_start < x_end ? 1 : -1;

    int32_t error = deltaX - deltaY;

    int32_t x = y_start - start_y;
    int32_t y = x_start;

    int32_t end_x = y_end - start_y;
    while((x != end_x || y != x_end) && (y != row))
    {
      const int32_t error2 = error * 2;
      if(error2 > -deltaY)
      {
        error -= deltaY;
        x += signX;
      }
      if(error2 < deltaX)
      {
        error += deltaX;
        y += signY;
      }
    }

    // If we found current line
    if(y == row)
    {
      // Go and draw line
      do
      {
        if((x >= 0) && (x < n)) buf[x] = color;
        const int32_t error2 = error * 2;
        if(error2 > -deltaY)
        {
          error -= deltaY;
          x += signX;
        }
        if(error2 < deltaX)
        {
          break;
        }
      }
      while((x != end_x) || (y != x_end));
    }
  }
}

// *****************************************************************************
// *****************************************************************************
// ***   Circle   **************************************************************
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// ***   Constructor   *********************************************************
// *****************************************************************************
Circle::Circle(int32_t x, int32_t y, int32_t r, color_t c, bool is_fill)
{
  SetParams(x, y, r, c, is_fill);
}

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void Circle::SetParams(int32_t x, int32_t y, int32_t r, color_t c, bool is_fill)
{
  // Lock object for changes
  LockVisObject();
  // Invalidate area for old position/size
  InvalidateObjArea();
  // Do changes
  color = c;
  radius = r;
  x_start = x - r;
  y_start = y - r;
  x_end = x + r;
  y_end = y + r;
  width = r*2;
  height = r*2;
  rotation = 0;
  fill = is_fill;
  // Invalidate area for new position/size
  InvalidateObjArea();
  // Unlock object after changes
  UnlockVisObject();
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void Circle::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
  // Draw only if needed
  if((line >= y_start) && (line <= y_end))
  {
    int32_t x = 0;
    int32_t y = radius;
    int32_t x0 = x_start + radius - start_x;
    int32_t y0 = y_start + radius;
    int32_t delta = 1 - 2 * radius;
    int32_t error = 0;
    bool line_drawed = false;

    while(y >= 0) 
    {
      if( (y0 + y == line) || (y0 - y == line) )
      {
        if(fill)
        {
          int32_t i = x0 - x;
          if(i < 0) i = 0;
          if(x0 + x < n) n = x0 + x;
          for(;i < n; i++)
          {
            buf[i] = color;
          }
          break;
        }
        else
        {
          int32_t xl = x0 - x;
          int32_t xr = x0 + x;
          if((xl > 0) && (xl < n)) buf[xl] = color;
          if((xr > 0) && (xr < n)) buf[xr] = color;
        }
        line_drawed = true;
      }
      else
      {
        if(line_drawed == true) break;
      }
      error = 2 * (delta + y) - 1;
      if(delta < 0 && error <= 0)
      {
        ++x;
        delta += 2 * x + 1;
        continue;
      }
      error = 2 * (delta - x) - 1;
      if(delta > 0 && error > 0)
      {
        --y;
        delta += 1 - 2 * y;
        continue;
      }
      ++x;
      delta += 2 * (x - y);
      --y;
    }
  }
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void Circle::DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y)
{
  if((row >= x_start) && (row <= x_end))
  {
    // Find start x position
    int32_t start = y_start - start_y;
    // Prevent write in memory before buffer
    if(start < 0) start = 0;
    // Find start x position
    int32_t end = y_end - start_y;
    // Prevent buffer overflow
    if(end > n) end = n;
    // Have sense draw only if end pointer in buffer
    if(end > 0)
    {
      for(int32_t i = start; i < end; i++) buf[i] = color;
    }
  }
}

// *****************************************************************************
// *****************************************************************************
// ***   Triangle   ****************************************************************
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// ***   Constructor   *********************************************************
// *****************************************************************************
Triangle::Triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, color_t c, bool is_fill)
{
  SetParams(x1, y1, x2, y2, x3, y3, c, is_fill);
}

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void Triangle::SetParams(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, color_t c, bool is_fill)
{
  // Lock object for changes
  LockVisObject();
  // Invalidate area for old position/size
  InvalidateObjArea();
  // Do changes
  color = c;
  fill = is_fill;
  x_start = (x1 < x2) ? (x1 < x3 ? x1 : x3) : (x2 < x3 ? x2 : x3);
  y_start = (y1 < y2) ? (y1 < y3 ? y1 : y3) : (y2 < y3 ? y2 : y3);
  x_end = (x1 > x2) ? (x1 > x3 ? x1 : x3) : (x2 > x3 ? x2 : x3);
  y_end = (y1 > y2) ? (y1 > y3 ? y1 : y3) : (y2 > y3 ? y2 : y3);
  width  = x_end - x_start;
  height = y_end - y_start;
  // Lines for draw
  lines[0].x1 = x1;
  lines[0].y1 = y1;
  lines[0].x2 = x2;
  lines[0].y2 = y2;
  lines[1].x1 = x1;
  lines[1].y1 = y1;
  lines[1].x2 = x3;
  lines[1].y2 = y3;
  lines[2].x1 = x2;
  lines[2].y1 = y2;
  lines[2].x2 = x3;
  lines[2].y2 = y3;
  rotation = 0;
  // Invalidate area for new position/size
  InvalidateObjArea();
  // Unlock object after changes
  UnlockVisObject();
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void Triangle::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
  // Draw only if needed
  if((line >= y_start) && (line <= y_end))
  {
    int32_t x_min = 10000;
    int32_t x_max = -1;
    // Find minimum and maximum x for given screen line
    for(uint8_t i = 0u; i < 3u; i++)
    {
      // Draw only if needed
      if(((line >= lines[i].y1) && (line <= lines[i].y2)) || ((line >= lines[i].y2) && (line <= lines[i].y1)))
      {
        const int32_t deltaX = abs(lines[i].x2 - lines[i].x1);
        const int32_t deltaY = abs(lines[i].y2 - lines[i].y1);
        const int32_t signX = lines[i].x1 < lines[i].x2 ? 1 : -1;
        const int32_t signY = lines[i].y1 < lines[i].y2 ? 1 : -1;

        int32_t error = deltaX - deltaY;

        int32_t x = lines[i].x1 - start_x;
        int32_t y = lines[i].y1;

        int32_t end_x = lines[i].x2 - start_x;
        // Go trought cycle until reach end of line or current line
        while((x != end_x || y != lines[i].y2) && (y != line))
        {
          const int32_t error2 = error * 2;
          if(error2 > -deltaY) 
          {
            error -= deltaY;
            x += signX;
          }
          if(error2 < deltaX) 
          {
            error += deltaX;
            y += signY;
          }
        }
        // If we found current line
        if(y == line)
        {
          // Go and draw line
          do
          {
            if((x >= 0) && (x < n))
            {
              if(fill == false) buf[x] = color;
              else
              {
                x_min = x_min > x ? x : x_min;
                x_max = x_max < x ? x : x_max;
              }
            }
            const int32_t error2 = error * 2;
            if(error2 > -deltaY) 
            {
              error -= deltaY;
              x += signX;
            }
            if(error2 < deltaX) 
            {
              break;
            }
          }
          while((x != end_x) || (y != lines[i].y2));
        }
      }
    }
    if(fill)
    {
      for(uint16_t i = x_min; i <= x_max; i++)
      {
        buf[i] = color;
      }
    }
  }
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void Triangle::DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y)
{
  // Not implemented
}
