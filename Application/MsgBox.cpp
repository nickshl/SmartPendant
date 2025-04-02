//******************************************************************************
//  @file MsgBox.cpp
//  @author Nicolai Shlapunov
//
//  @details MsgBox: User MsgBox Class, implementation
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
#include "MsgBox.h"

// *****************************************************************************
// ***   MsgBox Setup   ********************************************************
// *****************************************************************************
Result MsgBox::Setup(const char* title, const char* text, uint8_t title_scale)
{
  // Width of box
  static const uint32_t width = display_drv.GetScreenW();

  // MsgBox caption
  msg_box_caption.SetParams(title, 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
  msg_box_caption.SetScale(title_scale);
  msg_box_caption.Move((width - msg_box_caption.GetWidth()) / 2, BORDER_W * 2, false);
  msg_box_caption.SetList(list);
  msg_box_caption.Show(1u);

  // MsgBox text
  msg_box_text.SetParams(text, 0, 0, COLOR_WHITE, Font_10x18::GetInstance());
  msg_box_text.Move(BORDER_W * 2, msg_box_caption.GetEndY() + BORDER_W * 2, false);
  msg_box_text.SetList(list);
  msg_box_text.Show(1u);

  // Set box parameters
  box.SetParams(0, 0, width, msg_box_text.GetEndY() + BORDER_W*2, COLOR_BLACK, true);
  box.SetBorderWidth(BORDER_W);
  box.SetColor(COLOR_YELLOW);
  box.SetBackgroundColor(COLOR_BLACK);
  box.SetList(list);
  box.Show(0);

  // Set list params
  list.SetParams(0, (display_drv.GetScreenH() - box.GetHeight()) / 2, width, box.GetHeight());

  // Shadow box for whole screen
  shadowbox.SetParams(0, 0, display_drv.GetScreenW(), display_drv.GetScreenH());
  shadowbox.SetActive(true); // To cover all active elements

  // Left soft button
  left_btn.SetParams("Continue", 0, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3, display_drv.GetScreenW() / 2 - BORDER_W, Font_8x12::GetInstance().GetCharH() * 3, true);
  left_btn.SetCallback(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this);
  // Right soft button
  right_btn.SetParams("Cancel", display_drv.GetScreenW() - left_btn.GetWidth(), left_btn.GetStartY(), left_btn.GetWidth(), left_btn.GetHeight(), true);
  right_btn.SetCallback(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Set callback function   ***********************************************
// *****************************************************************************
void MsgBox::SetCallback(AppTask* task, CallbackPtr func, void* param)
{
  callback_task = task;
  callback_func = func;
  callback_param = param;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result MsgBox::Show(uint32_t z)
{
  shadowbox.Show(z);

  // SHow list
  list.Show(z+1);

  // Soft Buttons
  left_btn.Show(z+2);
  right_btn.Show(z+3);

  // Set callback handler for left and right buttons
  InputDrv::GetInstance().AddButtonsCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this, InputDrv::BTNM_LEFT | InputDrv::BTNM_RIGHT, btn_cble);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result MsgBox::Hide()
{
  // Delete buttons callback handler
  InputDrv::GetInstance().DeleteButtonsCallbackHandler(btn_cble);

  // Hide list
  list.Hide();
  // Hide shadow box
  shadowbox.Hide();

  // Soft Buttons
  left_btn.Hide();
  right_btn.Hide();

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Private: ProcessButtonCallback function   *****************************
// *****************************************************************************
Result MsgBox::ProcessButtonCallback(MsgBox* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    MsgBox& ths = *obj_ptr;

    // Process Left Soft Button
    if((ptr == &ths.left_btn) || (ptr == &ths.right_btn))
    {
      // Set selection
      if(ptr == &ths.left_btn) ths.selection = Result::RESULT_OK;
      else                     ths.selection = Result::ERR_CANCEL;
      // Hide itself and call callback
      ths.Hide();
      // If AppTask pointer provided
      if(ths.callback_task != nullptr)
      {
        // Call it to pass callback call to another task
        ths.callback_task->Callback(ths.callback_func, ths.callback_param, obj_ptr);
      }
      // If AppTask pointer is not provided, but callback function pointer provided
      else if(ths.callback_func != nullptr)
      {
        // Call it in Display task(mutex may be needed inside callback!)
        ths.callback_func(ths.callback_param, obj_ptr);
      }
      else
      {
        ; // Do nothing - MISRA rule
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
        // If button released
        if(btn.state == false)
        {
          // Call callback
          ProcessButtonCallback(obj_ptr, &ths.left_btn);
        }
      }
      // Right button - Reset values or send Stop command to GRBL if in movement
      else if(btn.btn == InputDrv::BTN_RIGHT)
      {
        // Set state of right soft button on the screen
        ths.right_btn.SetPressed(btn.state);
        // If button released
        if(btn.state == false)
        {
          // Call callback
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
