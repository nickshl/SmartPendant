//******************************************************************************
//  @file Tabs.cpp
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
#include "Tabs.h"

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void Tabs::SetParams(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t n)
{
  // This is an active object
  SetActive(true);
  // Set list params
  list.SetParams(x, y, w, h);
  // Set list for itself
  SetList(list);
  // Show itself for Action(). It will not show on display until list show.
  VisObject::Show(1000);

  // X and Y start coordinates of object is zero because of new list
  x_start = 0;
  y_start = 0;
  // X and Y end coordinates of object
  x_end =  w - 1;
  y_end =  h - 1;
  // Width and Height of object
  width = w;
  height = h;

  // Check input
  if(n > 8) n = 8;
  // Set tabs count
  tabs_cnt = n;
  // Calculate tab width
  tab_w = w / n;

  // TODO: border doubles on tab sides
  for(uint32_t i = 0; i < n; i++)
  {
    box[i].SetList(list);
    box[i].SetParams(tab_w*i, 0, tab_w,height, border_color, true);
    box[i].SetBorderWidth(1);
    box[i].SetBackgroundColor(tab_color);
    box[i].Show(0); // It will not show on display until list show
  }
  // Pressed tab - change inner color and remove bottom line
  tab.SetList(list);
  tab.SetParams(1, 1, tab_w - 2, height - 1, selected_tab_color, true);
  // Show selected tabs. It will not show on display until list show.
  tab.Show(1);
}

// *****************************************************************************
// ***   Public: SetImage   ****************************************************
// *****************************************************************************
void Tabs::SetImage(uint32_t tab, const ImageDesc& img_dsc)
{
  if(tab < NumberOf(img))
  {
    img[tab].SetList(list);
    img[tab].SetImage(img_dsc);
    img[tab].Move((list.GetWidth() / tabs_cnt) * tab + (list.GetWidth() / tabs_cnt - img[tab].GetWidth()) / 2, 0);
    img[tab].Show(3); // It will not show on display if list isn't show
  }
}

// *****************************************************************************
// ***   Public: SetText   *****************************************************
// *****************************************************************************
void Tabs::SetText(uint32_t tab, const char *str1, const char *str2, Font& font)
{
  if((tab < NumberOf(img)) && (str1 != nullptr))
  {
    // If no second string
    if(str2 == nullptr)
    {
      tab_cap[tab][0].SetList(list);
      tab_cap[tab][0].SetParams(str1, 0, 0, COLOR_WHITE, font);
      tab_cap[tab][0].Move((width / tabs_cnt) * tab + (width / tabs_cnt - tab_cap[tab][0].GetWidth()) / 2, (height - tab_cap[tab][0].GetHeight()) / 2);
      tab_cap[tab][0].Show(3); // It will not show on display if list isn't show
    }
    else
    {
      tab_cap[tab][0].SetList(list);
      tab_cap[tab][1].SetList(list);
      tab_cap[tab][0].SetParams(str1, 0, 0, COLOR_WHITE, font);
      tab_cap[tab][1].SetParams(str2, 0, 0, COLOR_WHITE, font);
      tab_cap[tab][0].Move((width / tabs_cnt) * tab + (width / tabs_cnt - tab_cap[tab][0].GetWidth()) / 2, (height - tab_cap[tab][0].GetHeight() - tab_cap[tab][1].GetHeight()) / 2);
      tab_cap[tab][1].Move((width / tabs_cnt) * tab + (width / tabs_cnt - tab_cap[tab][1].GetWidth()) / 2, tab_cap[tab][0].GetEndY() + 1);
      tab_cap[tab][0].Show(3); // It will not show on display if list isn't show
      tab_cap[tab][1].Show(3); // It will not show on display if list isn't show
    }
  }
}

// *****************************************************************************
// ***   SetBorder   ***********************************************************
// *****************************************************************************
void Tabs::SetBorder(color_t c)
{
  border_color = c;
//  // Invalidate area
//  InvalidateObjArea();
}

// *****************************************************************************
// ***   Public: SetSelectedTab   **********************************************
// *****************************************************************************
void Tabs::SetSelectedTab(uint32_t n)
{
  // If selected different tab and new value is correct
  if((n != selected_tab) && (n < tabs_cnt))
  {
    // Save selected tab
    selected_tab = n;
    // Move tab selector to appropriate position
    tab.SetParams(tab_w * selected_tab + 1, 1, tab_w - 2, list.GetHeight() - 1, selected_tab_color, true);
    // Invalidate area
    InvalidateObjArea();
  }
}

// *****************************************************************************
// ***   Set callback function   ***********************************************
// *****************************************************************************
void Tabs::SetCallback(AppTask* task, CallbackPtr func, void* param)
{
  callback_task = task;
  callback_func = func;
  callback_param = param;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result Tabs::Show(uint32_t z)
{
  // Show list and return result
  return list.Show(z);
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result Tabs::Hide()
{
  // Hide list and return result
  return list.Hide();
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void Tabs::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
 // Empty object - only for Action();
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void Tabs::DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y)
{
  // Empty object - only for Action();
}

// *****************************************************************************
// ***   Action   **************************************************************
// *****************************************************************************
void Tabs::Action(VisObject::ActionType action, int32_t tx, int32_t ty, int32_t tpx, int32_t tpy)
{
  // Switch for process action
  switch(action)
  {
    // Touch action
    case VisObject::ACT_TOUCH:  // Fall thru
    case VisObject::ACT_MOVEIN:
      break;

    // Untouch action
    case VisObject::ACT_UNTOUCH:
      // Calculate selected tab
      selected_tab = (tx - x_start) / tab_w;
      // Check overflow
      if(selected_tab >= tabs_cnt) selected_tab = tabs_cnt - 1;
      // Move tab selector to appropriate position
      tab.SetParams(tab_w * selected_tab + 1, 1, tab_w - 2, height - 1, selected_tab_color, true);
      // Invalidate area
      InvalidateObjArea();
      // Call callback
      if(callback_task != nullptr)
      {
        callback_task->Callback(callback_func, callback_param, this);
      }
      break;

    case VisObject::ACT_MOVEOUT:
      break;

    case VisObject::ACT_HOLD: // Intentional fall-trough
    case VisObject::ACT_MAX:
    default:
      break;
  }
}
