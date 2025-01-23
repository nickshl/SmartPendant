//******************************************************************************
//  @file TextBox.cpp
//  @author Nicolai Shlapunov
//
//  @details TextBox: User TextBox Class, implementation
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
#include "TextBox.h"

// *****************************************************************************
// ***   Setup   ***************************************************************
// *****************************************************************************
Result TextBox::Setup(int32_t x, int32_t y, int32_t w, int32_t h)
{
  Result result = Result::RESULT_OK;

  // Set list params
  VisList::SetParams(x, y, w, h);

  // Find how many visible lines do we have
  visible_cnt = h / Font_10x18::GetInstance().GetCharH();
  // We can;t have more visible lines than we have string objects
  if(visible_cnt > NumberOf(str)) visible_cnt = NumberOf(str);

  // Set strings parameters
  for(uint32_t i = 0u; i < visible_cnt; i++)
  {
    str[i].SetList(*this);
    str[i].SetParams(str_text[i], 0, Font_10x18::GetInstance().GetCharH() * i, COLOR_WHITE, Font_10x18::GetInstance());
  }

  // Set selection box parameters
  box.SetList(*this);
  box.SetParams(0, 0, VisList::GetWidth(), Font_10x18::GetInstance().GetCharH(), COLOR_BLUE, true);

  // Return result
  return result;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result TextBox::Show(uint32_t z)
{
  Result result = Result::RESULT_OK;

  // Scroll to current position
  Scroll(scroll_pos);
  // Show all lines
  for(uint32_t i = 0u; i < visible_cnt; i++)
  {
    str[i].SetString(str_text[i]);
    str[i].Show(1);
  }
  // Set selection box parameters
  box.SetParams(str[select_pos - scroll_pos].GetStartX(), str[select_pos - scroll_pos].GetStartY(), VisList::GetWidth(), str[select_pos - scroll_pos].GetHeight(), COLOR_BLUE, true);
  // Show selection box
  box.Show(0);
  // Show list
  VisList::Show(z);

  // Return result
  return result;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result TextBox::Hide()
{
  Result result = Result::RESULT_OK;

  // Hide list
  VisList::Hide();

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: SetText   *****************************************************
// *****************************************************************************
bool TextBox::SetText(const char* text)
{
  bool lines_fit = true;
  // Clear lines counter
  lines_cnt = 0;
  // Save text pointer
  p_text = text;
  // Clear scroll counter
  scroll_pos = 0;
  // Set scroll pointer
  p_scroll = text;
  // Pointer to text
  const char *ptr = p_text;
  // Count how many lines text have
  while(*ptr != '\0')
  {
    // Save pointer to calculate line length
    const char *start_ptr = ptr;
    // Increase lines count
    lines_cnt++;
    // Skip all characters until end of line or end of string
    while((*ptr != '\n') && (*ptr != '\r') && (*ptr != '\0')) ptr++;
    // Skip all CR LF symbols TODO:: handle multiple(empty lines)!
    while((*ptr == '\n') || (*ptr == '\r')) ptr++;
    // Check length, if line will not fit completley into line buffer - clear flag
    if((uint32_t)(ptr - start_ptr) > (NumberOf(str_text[0u]) - 1)) lines_fit = false;
  }
  // Select and scroll to first line
  Select(0);
  // Return flag
  return lines_fit;
}

// *****************************************************************************
// ***   Select   **************************************************************
// *****************************************************************************
Result TextBox::Select(int32_t n)
{
  Result result = Result::ERR_BAD_PARAMETER;

  // Check if requested position in range
  if((n >= 0) && (n < lines_cnt))
  {
    // Change select position only
    select_pos = n;

    // If selected position before scroll position - scroll to show selection
    if((select_pos < scroll_pos) || (select_pos == 0))
    {
      Scroll(select_pos);
    }

    // If selected position after scroll position - scroll to show selection
    if(select_pos > scroll_pos + (int32_t)visible_cnt - 1)
    {
      Scroll(select_pos - (visible_cnt - 1));
    }

    // Set selection box parameters
    box.SetParams(str[select_pos - scroll_pos].GetStartX(), str[select_pos - scroll_pos].GetStartY(), VisList::GetWidth(), str[select_pos - scroll_pos].GetHeight(), COLOR_BLUE, true);

    // Input parameter is ok
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Scroll   **************************************************************
// *****************************************************************************
Result TextBox::Scroll(int32_t n)
{
  Result result = Result::ERR_INVALID_ITEM;

  // We really need to scroll only if scroll position is different or if it zero
  if((scroll_pos != n) || (n == 0))
  {
    // Scroll position can be in range 0 ... lines_cnt - 1
    if(n >= lines_cnt) n = lines_cnt - 1;
    if(n <= 0) n = 0;
    // Calculate difference, if requested line is 0, the difference should be 0 too
    int32_t scroll_diff = n ? (n - scroll_pos) : 0;

    // Pointer to text
    const char *ptr = p_scroll;

    // Search first scrolled line forward
    if(scroll_diff > 0)
    {
      // Count how many lines text have
      while(scroll_diff && (*ptr != '\0'))
      {
        // Decrease scroll difference
        scroll_diff--;
        // Skip all characters until end of line or end of string
        while((*ptr != '\n') && (*ptr != '\r') && (*ptr != '\0')) ptr++;
        // Skip all CR LF symbols TODO:: handle multiple(empty lines)!
        while((*ptr == '\n') || (*ptr == '\r')) ptr++;
      }
    }
    else if(scroll_diff < 0) // Search first scrolled line backward
    {
      // Find CR or LF character of previous line
      while((*ptr != '\n') && (*ptr != '\r') && (ptr > p_text)) ptr--;
      // Count how many lines text have
      while(scroll_diff && (ptr > p_text))
      {
        // Increase scroll difference
        scroll_diff++;
        // Skip all CR LF symbols TODO:: handle multiple(empty lines)!
        while(((*ptr == '\n') || (*ptr == '\r')) && (ptr > p_text)) ptr--;
        // Skip all characters until end of line or end of string
        while(((*ptr != '\n') && (*ptr != '\r')) && (ptr > p_text)) ptr--;
      }
      // Since we ended up search on previous line CR or LF character - increase pointer by one
      if(ptr != p_text) ptr++;
    }
    else // if(scroll_diff == 0)
    {
      // Set scroll pointer to the text itself
      ptr = p_text;
    }
    // Update scroll pointer
    p_scroll = ptr;
    // Save requested scroll value
    scroll_pos = n;

    // Index of line
    uint32_t idx = 0u;
    // Copy text to all visible lines
    for(; idx < visible_cnt;)
    {
      // Skip CR LF TODO:: handle multiple(empty lines)!
      while((*ptr == '\n') || (*ptr == '\r')) ptr++;
      // If reached end of string - break the cycle
      if(*ptr == 0) break;
      // Copy text
      ptr = &ptr[Strncpy(str_text[idx], ptr, NumberOf(str_text[idx]))];
      // Increase line index
      idx++;
      // If we set at least one string
      result = Result::RESULT_OK;
    }
    // Clear remaining lines
    for(; idx < visible_cnt; idx++)
    {
      str_text[idx][0] = '\0';
    }
    // Invalidate area to update all strings on the display
    InvalidateObjArea();
  }
  else
  {
    // If we set at least one string
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: Strncpy function   *******************************************
// *****************************************************************************
uint32_t TextBox::Strncpy(char *dst, const char *src, uint32_t n)
{
  uint32_t i = 0u;
  // Copy string
  for(; i < n - 1; i++)
  {
    if((src[i] != '\0') && (src[i] != '\n') && (src[i] != '\r'))
    {
      dst[i] = src[i];
    }
    else
    {
      break;
    }
  }
  // Terminate string
  dst[i] = '\0';
  // Skip line to the end
  while((src[i] != '\0') && (src[i] != '\n') && (src[i] != '\r')) i++;
  // Return string length
  return i;
}
