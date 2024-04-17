//******************************************************************************
//  @file Header.cpp
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
#include "Header.h"

// *****************************************************************************
// ***   SetParams   ***********************************************************
// *****************************************************************************
void Header::SetParams(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t n)
{
  // This is an active object
  SetActive(true);

  // X and Y start coordinates of object is zero because of new list
  x_start = x;
  y_start = y;
  // X and Y end coordinates of object
  x_end =  w - 1;
  y_end =  h - 1;
  // Width and Height of object
  width = w;
  height = h;

  // Bottom line
  line_bottm.SetParams(x_start, y_end, x_end, y_end, COLOR_WHITE);

  // Left button
  btn_left.SetParams("<", x_start, y_start, height, height - 1, true);
  btn_left.SetFont(Font_12x16::GetInstance());
  btn_left.SetCallback(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessUiCallback), this);
  // Right button
  btn_right.SetParams(">", x_end - height, y_start, height, height - 1, true);
  btn_right.SetFont(Font_12x16::GetInstance());
  btn_right.SetCallback(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessUiCallback), this);

  // Set callback handler for left and right buttons
  InputDrv::GetInstance().AddButtonsCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this, InputDrv::BTNM_LEFT_UP | InputDrv::BTNM_RIGHT_UP, btn_cble);

  // Set box params
  box.SetParams(x_start, y_end + 1, width, display_drv.GetScreenH() - height, COLOR_DARKGREY, true);

  // Check input
  if(n > MAX_PAGES) n = MAX_PAGES;
  // Set pages count
  pages_cnt = n;
}

// *****************************************************************************
// ***   Public: SetImage   ****************************************************
// *****************************************************************************
void Header::SetImage(uint32_t page_idx, const ImageDesc& img_dsc)
{
  if(page_idx < NumberOf(img))
  {
    img[page_idx].SetImage(img_dsc);
    img[page_idx].Move(height + ((width - height * 2) - (img[page_idx].GetWidth() + str[page_idx].GetWidth() + str[page_idx].GetFontW())) / 2, (height - img[page_idx].GetHeight()) / 2);
    str[page_idx].Move(img[page_idx].GetEndX() + str[page_idx].GetFontW(), str[page_idx].GetStartY());
  }
}

// *****************************************************************************
// ***   Public: SetText   *****************************************************
// *****************************************************************************
void Header::SetText(uint32_t page_idx, const char *str_in, Font& font)
{
  if((page_idx < NumberOf(str)) && (str_in != nullptr))
  {
    str[page_idx].SetParams(str_in, 0, 0, COLOR_WHITE, font);
    str[page_idx].Move(height + ((width - height * 2) - str[page_idx].GetWidth()) / 2, (height - str[page_idx].GetHeight()) / 2);
    // Button for fast select
    button[page_idx].SetParams(str_in, height, height + (height * page_idx) + 4, width - (height * 2), height - 8, true);
    button[page_idx].SetFont(font);
    button[page_idx].SetCallback(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessUiCallback), this);
  }
}

// *****************************************************************************
// ***   SetBorder   ***********************************************************
// *****************************************************************************
void Header::SetBorder(color_t c)
{
//  border_color = c;
//  // Invalidate area
//  InvalidateObjArea();
}

// *****************************************************************************
// ***   Public: SetSelectedPage   *********************************************
// *****************************************************************************
void Header::SetSelectedPage(uint32_t n)
{
  // If selected different page and new value is correct
  if((n != selected_page) && (n < pages_cnt))
  {
    // Save selected tab
    selected_page = n;
    // If it on the screen - update header text
    if(IsShow()) Show(GetZ());
  }
}

// *****************************************************************************
// ***   Set callback function   ***********************************************
// *****************************************************************************
void Header::SetCallback(AppTask* task, CallbackPtr func, void* param)
{
  callback_task = task;
  callback_func = func;
  callback_param = param;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result Header::Show(uint32_t z)
{
  // Show itself for Action(). It will not show on display until list show.
  Result result = VisObject::Show(z);

  // Show lines
  line_bottm.Show(z);
  // Show buttons
  btn_left.Show(z+1);
  btn_right.Show(z+1);

  // Show header
  for(uint32_t i = 0; i < pages_cnt; i++)
  {
    if(i == selected_page)
    {
      img[i].Show(z);
      str[i].Show(z);
    }
    else
    {
      img[i].Hide();
      str[i].Hide();
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result Header::Hide()
{
  // Hide itself
  Result result = VisObject::Hide();

  // Hide lines
  line_bottm.Hide();
  // Hide buttons
  btn_left.Hide();
  btn_right.Hide();

  // Hide caption
  img[selected_page].Hide();
  str[selected_page].Hide();

  // Hide box
  box.Hide();

  // Hide buttons
  for(uint32_t i = 0; i < pages_cnt; i++)
  {
    button[i].Hide();
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void Header::DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t start_x)
{
 // Empty object - only for Action();
}

// *****************************************************************************
// ***   Put line in buffer   **************************************************
// *****************************************************************************
void Header::DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t start_y)
{
  // Empty object - only for Action();
}

// *****************************************************************************
// ***   Action   **************************************************************
// *****************************************************************************
void Header::Action(VisObject::ActionType action, int32_t tx, int32_t ty, int32_t tpx, int32_t tpy)
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
      // Avoid buttons area
      if((tx > x_start + height) && (tx < x_end - height))
      {
        if(box.IsShow())
        {
          // Hide everything
          Hide();
          // Show itself without box and buttons
          Show(GetZ());
        }
        else
        {
          // Show box
          box.Show(GetZ());
          // Show buttons
          for(uint32_t i = 0; i < pages_cnt; i++)
          {
            button[i].Show(GetZ() + 1);
          }
        }
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

// *****************************************************************************
// ***   Private: ProcessUiCallback function   *********************************
// *****************************************************************************
Result Header::ProcessUiCallback(Header* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    Header& ths = *obj_ptr;

    // Save selected page
    uint32_t prev_sel_page = ths.selected_page;

    if(ptr == &ths.btn_left)
    {
      // Switch left if it possible
      if(ths.selected_page) ths.selected_page--;

    }
    else if(ptr == &ths.btn_right)
    {
      // Switch right if it possible
      if(ths.selected_page < ths.pages_cnt - 1) ths.selected_page++;
    }
    else
    {
      // Check buttons
      for(uint32_t i = 0; i < ths.pages_cnt; i++)
      {
        if(ptr == &ths.button[i])
        {
          // Switch right if it possible
          ths.selected_page = i;
          // Hide everything
          ths.Hide();
        }
      }
    }

    // If selected tab changed
    if(ths.selected_page != prev_sel_page)
    {
      // Hide box if it shown
      if(ths.box.IsShow())
      {
        ths.Hide();
      }
      // Update caption
      ths.Show(ths.GetZ());
      // Call callback
      if(ths.callback_task != nullptr)
      {
        ths.callback_task->Callback(ths.callback_func, ths.callback_param, obj_ptr);
      }
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ProcessButtonCallback function   *****************************
// *****************************************************************************
Result Header::ProcessButtonCallback(Header* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    Header& ths = *obj_ptr;
    // Get pressed button
    InputDrv::ButtonCallbackData btn = *((InputDrv::ButtonCallbackData*)ptr);

    // UI Button pointer
    UiButton *ui_btn = nullptr;

    // Find button pointer
    if(btn.btn == InputDrv::BTN_LEFT_UP)       ui_btn = &ths.btn_left;
    else if(btn.btn == InputDrv::BTN_RIGHT_UP) ui_btn = &ths.btn_right;
    else; // Do nothing - MISRA rule

    // If button object found
    if(ui_btn != nullptr)
    {
      // If button pressed
      if(btn.state == true)
      {
        // Press button on the screen
        ui_btn->SetPressed(true);
      }
      else // Released
      {
        // Release button on the screen
        ui_btn->SetPressed(false);
        // And call callback
        ths.ProcessUiCallback(obj_ptr, ui_btn);
      }
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}
