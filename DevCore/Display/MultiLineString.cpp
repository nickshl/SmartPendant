//******************************************************************************
//  @file MultiLineString.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: MultiLineString Visual Object Class, implementation
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
#include "MultiLineString.h"

#include <cstring> // for strlen()
#include <cstring>
#include <stdarg.h>
#include <stdio.h>

// *****************************************************************************
// ***   Constructor   *********************************************************
// *****************************************************************************
MultiLineString::MultiLineString(const char* str, int32_t x, int32_t y, color_t tc, Font& font)
{
  SetParams(str, x, y, tc, font);
}

// *****************************************************************************
// ***   Constructor   *********************************************************
// *****************************************************************************
MultiLineString::MultiLineString(const char* str, int32_t x, int32_t y, color_t tc, color_t bgc, Font& font)
{
  SetParams(str, x, y, tc, bgc, font);
}

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void MultiLineString::SetParams(const char* str, int32_t x, int32_t y, color_t tc, Font& font)
{
  // Lock object for changes
  LockVisObject();
  // Invalidate area for old string(needed if old string longer than new)
  InvalidateObjArea();
  // Do changes
  string = str;
  x_start = x;
  y_start = y;
  txt_color = tc;
  bg_color = 0;
  font_ptr = &font;
  transpatent_bg = true;
  line_height = font.GetCharH() * scale + spacing;
  width = font.GetCharW() * GetLongestLineLength(str) * scale;
  height = (line_height * GetStringCount(str)) - spacing;
  x_end = x + width - 1;
  y_end = y + height - 1;
  rotation = 0;
  // Clear current line to set new pointer inside draw function.
  str_line = -1;
  // Invalidate area for new string(needed if new string longer than old)
  InvalidateObjArea();
  // Unlock object after changes
  UnlockVisObject();
}

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void MultiLineString::SetParams(const char* str, int32_t x, int32_t y, color_t tc, color_t bgc, Font& font)
{
  // Lock object for changes
  LockVisObject();
  // Invalidate area for old string(needed if old string longer than new)
  InvalidateObjArea();
  // Do changes
  string = str;
  x_start = x;
  y_start = y;
  txt_color = tc;
  bg_color = bgc;
  font_ptr = &font;
  transpatent_bg = false;
  line_height = font.GetCharH() * scale + spacing;
  width = font.GetCharW() * GetLongestLineLength(str) * scale;
  height = (line_height * GetStringCount(str)) - spacing;
  x_end = x + width - 1;
  y_end = y + height - 1;
  rotation = 0;
  // Clear current line to set new pointer inside draw function.
  str_line = -1;
  // Invalidate area for new string(needed if new string longer than old)
  InvalidateObjArea();
  // Unlock object after changes
  UnlockVisObject();
}

// *****************************************************************************
// ***   SetColor   ************************************************************
// *****************************************************************************
void MultiLineString::SetColor(color_t tc, color_t bgc, bool is_trnsp)
{
  // Check if parameters changed
  if((tc != txt_color) || (bgc != bg_color) || (is_trnsp != transpatent_bg))
  {
    // No Lock/Unlock since it will not break anything
    txt_color = tc;
    bg_color = bgc;
    transpatent_bg = is_trnsp;
    // Invalidate area
    InvalidateObjArea();
  }
}

// *****************************************************************************
// ***   Public: SetFont   *****************************************************
// *****************************************************************************
void MultiLineString::SetFont(Font& font)
{
  // Check if font changed
  if(&font != font_ptr)
  {
    // Lock object for changes
    LockVisObject();
    // Invalidate area for old string(needed if old string font bigger than new one)
    InvalidateObjArea();
    // Do changes
    font_ptr = &font;
    line_height = font.GetCharH() * scale + spacing;
    width = font.GetCharW() * GetLongestLineLength(string) * scale;
    height = (line_height * GetStringCount(string)) - spacing;
    x_end = x_start + width - 1;
    y_end = y_start + height - 1;
    // Invalidate area for new string(needed if new string font bigger than old one)
    InvalidateObjArea();
    // Unlock object after changes
    UnlockVisObject();
  }
}

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void MultiLineString::SetScale(uint8_t s)
{
  // Scale have to be positive number
  if((s != scale) && (s > 0u))
  {
    // Lock object for changes
    LockVisObject();
    // Invalidate area for old string(needed if old string scale bigger than new one)
    InvalidateObjArea();
    // Do changes
    scale = s;
    line_height = GetFontH() * scale + spacing;
    width = GetFontW() * GetLongestLineLength(string) * scale;
    height = (line_height * GetStringCount(string)) - spacing;
    x_end = x_start + width - 1;
    y_end = y_start + height - 1;
    // Invalidate area for new string(needed if new string scale bigger than old one)
    InvalidateObjArea();
    // Unlock object after changes
    UnlockVisObject();
  }
}

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void MultiLineString::SetSpacing(uint8_t s)
{
  if(s != spacing)
  {
    // Lock object for changes
    LockVisObject();
    // Invalidate area for old string(needed if old string scale bigger than new one)
    InvalidateObjArea();
    // Do changes
    spacing = s;
    line_height = GetFontH() * scale + spacing;
    height = (line_height * GetStringCount(string)) - spacing;
    y_end = y_start + height - 1;
    // Invalidate area for new string(needed if new string scale bigger than old one)
    InvalidateObjArea();
    // Unlock object after changes
    UnlockVisObject();
  }
}

// *****************************************************************************
// ***   SetString   ***********************************************************
// *****************************************************************************
void MultiLineString::SetString(const char* str, bool force)
{
  // Lock object for changes
  LockVisObject();
  // Ignore if same string set again
  if((string != str) || force)
  {
    // Invalidate area for old string(needed if old string longer than new)
    InvalidateObjArea();
    // Set new pointer to string
    string = str;
    // Clear current line to set new pointer inside draw function.
    str_line = -1;
    // Recalculate line height
    line_height = scale * GetFontH() + spacing;
    // Recalculate width
    width = GetFontW() * GetLongestLineLength(string) * scale;
    // Recalculate height. Spacing exist between lines only, so there one less spacing than lines.
    height = (line_height * GetStringCount(string)) - spacing;
    // Recalculate y_end
    y_end = y_start + height - 1;
    // Update string X end coordinate
    x_end = x_start + width - 1;
    // Invalidate area for new string(needed if new string longer than old)
    InvalidateObjArea();
  }
  // Unlock object after changes
  UnlockVisObject();
}

// *****************************************************************************
// ***   SetString   ***********************************************************
// *****************************************************************************
void MultiLineString::SetString(char* buf, uint32_t len, const char* format, ...)
{
  // Lock object for changes
  LockVisObject();
  // Invalidate area for old string(needed if old string longer than new)
  InvalidateObjArea();
  // Argument list
  va_list arglist;
  // Create string
  va_start(arglist, format);
  vsnprintf(buf, len, format, arglist);
  va_end(arglist);
  // Set new pointer to string
  string = buf;
  // Clear current line to set new pointer inside draw function.
  str_line = -1;
  // Recalculate line height
  line_height = scale * GetFontH() + spacing;
  // Recalculate width
  width = GetFontW() * GetLongestLineLength(string) * scale;
  // Recalculate height. Spacing exist between lines only, so there one less spacing than lines.
  height = (line_height * GetStringCount(string)) - spacing;
  // Recalculate y_end
  y_end = y_start + height - 1;
  // Calculate new x_end
  x_end = x_start + width - 1;
  // Invalidate area for new string(needed if new string longer than old)
  InvalidateObjArea();
  // Unlock object after changes
  UnlockVisObject();
}

// *****************************************************************************
// ***   Public: SetAlignment   ************************************************
// *****************************************************************************
void MultiLineString::SetAlignment(alignment_t a)
{
  // If align changed
  if(a != alignment)
  {
    // Update align
    alignment = a;
    // Invalidate area for update
    InvalidateObjArea();
  }
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void MultiLineString::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
  // Draw only if needed
  if((line >= y_start) && (line <= y_end) && (string != nullptr) && (font_ptr != nullptr))
  {
    int32_t txt_line = ((line - y_start)) / line_height;
    // Current symbol X position
    int32_t x = x_start - start_x;
    // Calculate font line to figure out spacing
    uint32_t font_line = ((line - y_start - line_height * txt_line) / scale);
    // Number of bytes need skipped for draw line
    uint32_t skip_bytes = font_line * GetFontBytePerChar() / GetFontH();
    // Pointer to string. Will increment for get characters.
    const char* str = str_ptr;

    // Optimization: instead do it all the time, we done it once per text line.
    if(txt_line != str_line)
    {
      // Store current line
      str_line = txt_line;
      // Set pointer to the beginning of text
      str = string;
      // Find line
      while(txt_line && (*str != '\0'))
      {
        if(*str == '\n')
        {
          txt_line--;
        }
        str++;
      }
      // Store line pointer
      str_ptr = str;
      // Find string length
      str_len = GetStringLength(str);
    }

    // Calculate alignment
    if(alignment == CENTER)
    {
      int32_t len = str_len * GetFontW() * scale;
      x += (width - len) / 2;
    }
    else if(alignment == RIGHT)
    {
      int32_t len = str_len * GetFontW() * scale;
      x += width - len;
    }
    else
    {
      ; // Do nothing
    }

    // Process spacing
    if(font_line >= GetFontH())
    {
      if(transpatent_bg == false)
      {
        // While we have symbols
        while((*str != '\0') && (*str != '\n'))
        {
          if((x >= 0) && (x < n))
          {
            buf[x++] = bg_color;
          }
        }
      }
    }
    else // Process text
    {
      // While we have symbols
      while((*str != '\0') && (*str != '\n'))
      {
        uint32_t b = 0;
        uint32_t w = 0;
        // Get pointer to character data
        const uint8_t* char_ptr = font_ptr->GetCharGataPtr(*str);
        // Get all symbol line
        for(uint32_t i = 0; i < GetFontBytePerChar() / GetFontH(); i++)
        {
          b |= char_ptr[skip_bytes + i] << (i*8);
        }
        // Output symbol line
        while(w < GetFontW())
        {
          for(uint8_t i = 0u; i < scale; i++)
          {
            // Put color in buffer only if visible
            if((x >= 0) && (x < n))
            {
              if((b&1) == 1)
              {
                buf[x] = txt_color;
              }
              else if(transpatent_bg == false)
              {
                buf[x] = bg_color;
              }
              else
              {
                // Empty statement
              }
            }
            x++;
          }
          b >>= 1;
          w++;
        }
        str++;
      }
    }
  }
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void MultiLineString::DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y)
{
  // Not implemented
}

// *****************************************************************************
// ***   Private: GetStringCount   *********************************************
// *****************************************************************************
uint32_t MultiLineString::GetStringCount(const char* str)
{
  uint32_t cnt = 1u;

  if(str != nullptr)
  {
    while((*str != '\0'))
    {
      if(*str == '\n') cnt++;
      str++;
    }
  }

  return cnt;
}

// *****************************************************************************
// ***   Private: GetStringLength   ********************************************
// *****************************************************************************
uint32_t MultiLineString::GetStringLength(const char* str)
{
  uint32_t cnt = 0u;

  if(str != nullptr)
  {
    while((str[cnt] != '\0') && (str[cnt] != '\n'))
    {
      cnt++;
    }
  }

  return cnt;
}

// *****************************************************************************
// ***   Private: GetLongestLineLength   ***************************************
// *****************************************************************************
uint32_t MultiLineString::GetLongestLineLength(const char* str)
{
  uint32_t cnt = 0u;

  if(str != nullptr)
  {
    while(*str != '\0')
    {
      while(*str == '\n') str++;
      uint32_t len = GetStringLength(str);
      if(len > cnt) cnt = len;
      str += len;
    }
  }

  return cnt;
}
