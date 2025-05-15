//******************************************************************************
//  @file Menu.cpp
//  @author Nicolai Shlapunov
//
//  @details Menu: User Menu Class, implementation
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
#include "Menu.h"

// *****************************************************************************
// ***   Public: Setup   *******************************************************
// *****************************************************************************
Result Menu::Setup(MenuItem* in_ptr, int32_t in_cnt, int32_t x, int32_t y, int32_t w, int32_t h)
{
  Result result = Result::ERR_NULL_PTR;

  ptr = in_ptr;
  cnt = in_cnt;
  result = Setup(x, y, w, h);

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: Setup   *******************************************************
// *****************************************************************************
Result Menu::Setup(int32_t x, int32_t y, int32_t w, int32_t h)
{
  Result result = Result::ERR_NULL_PTR;

  // Set list params
  list.SetParams(x, y, w, h);

  // Check pointer
  if(ptr != nullptr)
  {
    for(int32_t i = 0u; i < cnt; i++)
    {
      ptr[i].str.SetList(list);
      ptr[i].str.SetParams(ptr[i].text, 0, Font_10x18::GetInstance().GetCharH() * i, COLOR_WHITE, Font_10x18::GetInstance());
    }

    // Set selection box parameters
    box.SetList(list);
    box.SetParams(ptr[0].str.GetStartX(), ptr[0].str.GetStartY(), w, ptr[0].str.GetHeight(), COLOR_BLUE, true);

    // All good
    result = Result::RESULT_OK;
  }

  // Width depends how many buttons we have
  int32_t btn_width = display_drv.GetScreenW() / 2 - BORDER_W;

  // Left soft button
  left_btn.SetParams("Select", 0, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3, btn_width, Font_8x12::GetInstance().GetCharH() * 3, true);
  left_btn.SetCallback(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this);
  // Right soft button
  right_btn.SetParams("Cancel", display_drv.GetScreenW() - btn_width, left_btn.GetStartY(), btn_width, left_btn.GetHeight(), true);
  right_btn.SetCallback(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this);

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: SetCount   ****************************************************
// *****************************************************************************
void Menu::SetCount(int32_t in_cnt)
{
  // If new count less than current, we have to hide all excessive lines
  for(int32_t i = in_cnt; i < cnt; i++)
  {
    ptr[i].str.Hide();
  }
  // Save count
  cnt = in_cnt;
  // Correct selected line if needed
  if(cur_pos >= cnt)
  {
    cur_pos = 0;
    // Set selection box parameters
    box.SetParams(ptr[cur_pos].str.GetStartX(), ptr[cur_pos].str.GetStartY(), list.GetWidth(), ptr[cur_pos].str.GetHeight(), COLOR_BLUE, true);
  }
  // Show all lines
  for(int32_t i = 0u; i < cnt; i++)
  {
    ptr[i].str.Show(1);
  }
}

// *****************************************************************************
// ***   Public: SetCallback   *************************************************
// *****************************************************************************
void Menu::SetCallback(AppTask* task_in, void* param_in, CallbackPtr func_ent_in, CallbackPtr func_esc_in)
{
  task = task_in;
  param = param_in;
  func_ent = func_ent_in;
  func_esc = func_esc_in;
}

// *****************************************************************************
// ***   Public: CreateString   ************************************************
// *****************************************************************************
Result Menu::CreateString(MenuItem& item, const char* str1, const char* str2)
{
  // Buffer for format string
  char fmt[16] = {0};
  // Find total length that can be used
  int32_t total_len = list.GetWidth() / Font_10x18::GetInstance().GetCharW();
  // Find length
  int32_t len1 = strlen(str1);
  // Find remaining available characters after str1 is placed
  int32_t remaining = total_len - len1;
  // Create format string
  if(remaining > 0)
  {
    snprintf(fmt, sizeof(fmt), "%%s%%%lds", remaining);
  }
  else
  {
    snprintf(fmt, sizeof(fmt), "%%s");
  }
  // Set new string
  item.str.SetString(item.text, (item.n > (uint32_t)total_len) ? total_len + 1 : item.n, fmt, str1, str2);

  // Return result
  return Result::RESULT_OK;
}

// *************************************************************************
// ***   Public: CreateString   ********************************************
// *************************************************************************
Result Menu::CreateString(int32_t item, const char* str1, const char* str2)
{
  Result result = Result::ERR_INVALID_ITEM;

  // Check if requested item exists
  if((item >= 0) && (item < cnt))
  {
    result = CreateString(ptr[item], str1, str2);
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: Show   ********************************************************
// *****************************************************************************
Result Menu::Show(uint32_t z)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(ptr != nullptr)
  {
    // Show selection box
    box.Show(0);
    // Show all lines
    for(int32_t i = 0u; i < cnt; i++)
    {
      ptr[i].str.Show(1);
    }

    // Show list
    list.Show(z);

    // Set selection box parameters
    box.SetParams(ptr[cur_pos].str.GetStartX(), ptr[cur_pos].str.GetStartY(), list.GetWidth(), ptr[cur_pos].str.GetHeight(), COLOR_BLUE, true);

    // Soft Buttons
    left_btn.Show(z);
    right_btn.Show(z);

    // Set encoder callback handler
    InputDrv::GetInstance().AddEncoderCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessEncoderCallback), this, enc_cble);
    // Set callback handler for left and right buttons
    InputDrv::GetInstance().AddButtonsCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this, InputDrv::BTNM_LEFT | InputDrv::BTNM_RIGHT, btn_cble);

    // All good
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: Hide   ********************************************************
// *****************************************************************************
Result Menu::Hide()
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(ptr != nullptr)
  {
    // Delete encoder callback handler
    InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);
    // Delete buttons callback handler
    InputDrv::GetInstance().DeleteButtonsCallbackHandler(btn_cble);

    // Show selection box
    box.Hide();
    // Show all lines
    for(int32_t i = 0u; i < cnt; i++)
    {
      ptr[i].str.Hide();
    }

    // Hide list
    list.Hide();

    // Soft Buttons
    left_btn.Hide();
    right_btn.Hide();

    // All good
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ProcessEncoderCallback function   ****************************
// *****************************************************************************
Result Menu::ProcessEncoderCallback(Menu* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    Menu& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    // Change position
    ths.cur_pos += enc_val;
    // Check new value
    if(ths.cur_pos < 0) ths.cur_pos = 0;
    if(ths.cur_pos >= ths.cnt) ths.cur_pos = ths.cnt - 1;
    // Set selection box parameters
    ths.box.SetParams(ths.ptr[ths.cur_pos].str.GetStartX(), ths.ptr[ths.cur_pos].str.GetStartY(), ths.list.GetWidth(), ths.ptr[ths.cur_pos].str.GetHeight(), COLOR_BLUE, true);

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ProcessButtonCallback function   *****************************
// *****************************************************************************
Result Menu::ProcessButtonCallback(Menu* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    Menu& ths = *obj_ptr;

    // Process Left Soft Button
    if(ptr == &ths.left_btn)
    {
      // cur_pos can be -1 if there no any menu items
      if(ths.cur_pos >= 0)
      {
        // Call callback
        if(ths.task != nullptr)
        {
          ths.task->Callback(ths.func_ent, ths.param, (void*)ths.cur_pos);
        }
        else if(ths.func_ent != nullptr)
        {
          ths.func_ent(ths.param, (void*)ths.cur_pos);
        }
        else
        {
          ; // Do nothing - MISRA rule
        }
      }
    }
    // Process Right Soft Button
    else if(ptr == &ths.right_btn)
    {
      // cur_pos can be -1 if there no any menu items
      if(ths.cur_pos >= 0)
      {
        // Call callback
        if(ths.task != nullptr)
        {
          ths.task->Callback(ths.func_esc, ths.param, (void*)ths.cur_pos);
        }
        else if(ths.func_esc != nullptr)
        {
          ths.func_esc(ths.param, (void*)ths.cur_pos);
        }
        else
        {
          ; // Do nothing - MISRA rule
        }
      }
    }
    else // Process physical buttons
    {
      // Get pressed button
      InputDrv::ButtonCallbackData btn = *((InputDrv::ButtonCallbackData*)ptr);

      // Left button
      if(btn.btn == InputDrv::BTN_LEFT)
      {
        // Set state of left soft button on the screen
        ths.left_btn.SetPressed(btn.state);
        // If button pressed
        if(btn.state == true)
        {
          // Change color to indicate press
          ths.box.SetColor(COLOR_GREEN);
        }
        else // Released
        {
          // Change color back
          ths.box.SetColor(COLOR_BLUE);
          // And call callback
          ProcessButtonCallback(obj_ptr, &ths.left_btn);
        }
      }
      // Right button - Reset values or send Stop command to GRBL if in movement
      else if(btn.btn == InputDrv::BTN_RIGHT)
      {
        // Set state of right soft button on the screen
        ths.right_btn.SetPressed(btn.state);
        // If button pressed
        if(btn.state == true)
        {
          // Change color to indicate press
          ths.box.SetColor(COLOR_RED);
        }
        else // Released
        {
          // Change color back
          ths.box.SetColor(COLOR_BLUE);
          // And call callback
          ProcessButtonCallback(obj_ptr, &ths.right_btn);
        }
      }
      else
      {
        ; // Do nothing - MISRA rule
      }
    }

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}
