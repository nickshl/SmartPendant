//******************************************************************************
//  @file ChangeValueBox.cpp
//  @author Nicolai Shlapunov
//
//  @details ChangeValueBox: User ChangeValueBox Class, implementation
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
#include "ChangeValueBox.h"

// *****************************************************************************
// ***   ChangeValueBox Setup   ************************************************
// *****************************************************************************
Result ChangeValueBox::Setup(const char* title, const char* units, int32_t val, int32_t min, int32_t max, uint32_t point_pos, uint8_t title_scale)
{
  // Clear items pointer
  items = nullptr;

  // Width of box
  static const uint32_t width = display_drv.GetScreenW();

  // Value caption
  value_name.SetParams(title, 0, 0, COLOR_WHITE, Font_12x16::GetInstance());
  value_name.SetScale(title_scale);
  value_name.Move((width - value_name.GetWidth()) / 2, BORDER_W * 2, false);
  value_name.SetList(list);
  value_name.Show(1u);

  // Value position
  value_dw.SetParams(BORDER_W * 2, value_name.GetEndY() + BORDER_W*2, width - BORDER_W * 4,  Font_8x12::GetInstance().GetCharH() * 5u, 13u, point_pos);
  value_dw.SetBorder(BORDER_W, COLOR_GREEN);
  value_dw.SetDataFont(Font_8x12::GetInstance(), 2u);
  value_dw.SetLimits(min, max);
  value_dw.SetNumber(val);
  value_dw.SetUnits(units, DataWindow::BOTTOM_RIGHT);
  value_dw.SetActive(false);
  value_dw.SetList(list);
  value_dw.Show(1u);

  // Scale buttons
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    // Calculate scale button width
    uint32_t scale_btn_w = (value_dw.GetWidth() - BORDER_W * (NumberOf(scale_btn) - 1u)) / NumberOf(scale_btn);
    // Create button text
    GrblComm::GetInstance().ValueToString(scale_str[i], sizeof(scale_str[i]), scale_val[i], Power(10, point_pos));
    // Add units to the string
    snprintf(scale_str[i] + strlen(scale_str[i]), sizeof(scale_str[i]) - strlen(scale_str[i]), "\n%s", units);
    // Set button parameters
    scale_btn[i].SetParams(scale_str[i], value_dw.GetStartX() + i * (scale_btn_w + BORDER_W), value_dw.GetEndY() + BORDER_W*2, scale_btn_w, Font_8x12::GetInstance().GetCharH() * 5, true);
    scale_btn[i].SetCallback(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this);
    scale_btn[i].SetPressed(false);
    scale_btn[i].SetSpacing(3u);
    scale_btn[i].SetList(list);
    scale_btn[i].Show(1u);
  }

  // Since we have 3 buttons, any point position greater than 2 should set highest button
  if(point_pos > 2) point_pos = 2;
  // Set scale
  scale = Power(10, point_pos);
  // Set corresponded button pressed
  scale_btn[point_pos].SetPressed(true);

  // Set box parameters
  box.SetParams(0, 0, width, scale_btn[0u].GetEndY() + BORDER_W*2, COLOR_BLACK, true);
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
  left_btn.SetParams("Ok", 0, display_drv.GetScreenH() - Font_8x12::GetInstance().GetCharH() * 3, display_drv.GetScreenW() / 2 - BORDER_W, Font_8x12::GetInstance().GetCharH() * 3, true);
  left_btn.SetCallback(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this);
  // Right soft button
  right_btn.SetParams("Cancel", display_drv.GetScreenW() - left_btn.GetWidth(), left_btn.GetStartY(), left_btn.GetWidth(), left_btn.GetHeight(), true);
  right_btn.SetCallback(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   ChangeValueBox Setup   ************************************************
// *****************************************************************************
Result ChangeValueBox::Setup(const char* title, const char* const* itms, int32_t n, int32_t val, uint8_t title_scale)
{
  // Use main setup to create box and value caption
  Setup(title, "", val, 0, n - 1, 0, title_scale);

  // Save items
  items = itms;
  items_cnt = n;

  // Set string instead value
  value_dw.SetString(itms[val]);

  // No scale buttons for items list, so hide them
  for(uint32_t i = 0u; i < NumberOf(scale_btn); i++)
  {
    scale_btn[i].Hide();
  }

  // Set box parameters to make it smaller
  box.SetParams(0, 0, box.GetWidth(), value_dw.GetEndY() + BORDER_W*2, COLOR_BLACK, true);
  box.SetBorderWidth(BORDER_W);
  box.SetColor(COLOR_YELLOW);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Set callback function   ***********************************************
// *****************************************************************************
void ChangeValueBox::SetCallback(AppTask* task, CallbackPtr func, void* param)
{
  callback_task = task;
  callback_func = func;
  callback_param = param;
}

// *****************************************************************************
// ***   Show   ****************************************************************
// *****************************************************************************
Result ChangeValueBox::Show(uint32_t z)
{
  shadowbox.Show(z);

  // SHow list
  list.Show(z+1);

  // Soft Buttons
  left_btn.Show(z+2);
  right_btn.Show(z+3);

  // Set encoder callback handler
  InputDrv::GetInstance().AddEncoderCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessEncoderCallback), this, enc_cble);
  // Set callback handler for all buttons
  InputDrv::GetInstance().AddButtonsCallbackHandler(AppTask::GetCurrent(), reinterpret_cast<CallbackPtr>(ProcessButtonCallback), this, InputDrv::BTNM_ALL, btn_cble);

  // All good
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Hide   ****************************************************************
// *****************************************************************************
Result ChangeValueBox::Hide()
{
  // Delete encoder callback handler
  InputDrv::GetInstance().DeleteEncoderCallbackHandler(enc_cble);
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

// *************************************************************************
// ***   Private: ProcessEncoderCallback function   ************************
// *************************************************************************
Result ChangeValueBox::ProcessEncoderCallback(ChangeValueBox* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ChangeValueBox& ths = *obj_ptr;
    // Cast pointer itself to integer value
    int32_t enc_val = (int32_t)ptr;

    // Process it
    if(enc_val != 0)
    {
      // Calculate & set new value number
      ths.value_dw.SetNumber(ths.value_dw.GetNumber() + enc_val * ths.scale);
      // If it enum list - set string according to value
      if(ths.items != nullptr)
      {
        ths.value_dw.SetString(&ths.items[ths.value_dw.GetNumber()][0]);
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
Result ChangeValueBox::ProcessButtonCallback(ChangeValueBox* obj_ptr, void* ptr)
{
  Result result = Result::ERR_NULL_PTR;

  // Check pointer
  if(obj_ptr != nullptr)
  {
    // Cast pointer to "this". Since we can't use non-static members as callback,
    // we have to provide pinter to object.
    ChangeValueBox& ths = *obj_ptr;

    // Process Left and Right Soft Buttons
    if((ptr == &ths.left_btn) || (ptr == &ths.right_btn))
    {
      // Only if left button is pressed - want to apply value
      ths.is_ok_pressed = (ptr == &ths.left_btn) ? true : false;
      // Hide itself and call callback to show that value should be saved
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
    // Process scale buttons
    else if(ptr == &ths.scale_btn[0])
    {
      ths.scale = 1;
    }
    else if(ptr == &ths.scale_btn[1])
    {
      ths.scale = 10;
    }
    else if(ptr == &ths.scale_btn[2])
    {
      ths.scale = 100;
    }
    else // Process physical buttons
    {
      // Get pressed button
      InputDrv::ButtonCallbackData btn = *((InputDrv::ButtonCallbackData*)ptr);

      // Left button
      if(btn.btn == InputDrv::BTN_LEFT)
      {
        // Set state of "Enter" button on the screen
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
        // Set state of  "Cancel" button on the screen
        ths.right_btn.SetPressed(btn.state);
        // If button released
        if(btn.state == false)
        {
          // Call callback
          ProcessButtonCallback(obj_ptr, &ths.right_btn);
        }
      }
      else // Any other button - do nothing
      {
        ; // Do nothing - MISRA rule
      }
    }

    // Update objects on a screen
    ths.UpdateObjects();

    // Set ok result
    result = Result::RESULT_OK;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: Update function   ********************************************
// *****************************************************************************
void ChangeValueBox::UpdateObjects(void)
{
  // Set pressed state for selected scale button and unpressed for unselected ones
  scale_btn[0u].SetPressed(scale == 1);
  scale_btn[1u].SetPressed(scale == 10);
  scale_btn[2u].SetPressed(scale == 100);
}

// *****************************************************************************
// ***   Private: Power function   *********************************************
// *****************************************************************************
int32_t ChangeValueBox::Power(int32_t val, uint32_t power)
{
  // Result - 1 by default to handle 0 power
  int32_t result = 1;
  // If power isn't zero
  if(power != 0)
  {
    // Set result to value
    result = val;
    // And cycle until get power
    for(uint32_t i = 1u; i < power; i++) result *= val;
  }
  return result;
}
