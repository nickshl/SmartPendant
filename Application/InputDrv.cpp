//******************************************************************************
//  @file InputDrv.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: Input Driver Class, implementation
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
#include "InputDrv.h"

#include <cstdlib>

// *****************************************************************************
// ***   Button callback data   ************************************************
// *****************************************************************************
InputDrv::ButtonCallbackData InputDrv::btn_callback_data[BTN_MAX][2u] =
{
    {{InputDrv::BTN_LEFT, false},       {InputDrv::BTN_LEFT, true}},
    {{InputDrv::BTN_RIGHT, false},      {InputDrv::BTN_RIGHT, true}},
    {{InputDrv::BTN_LEFT_UP, false},    {InputDrv::BTN_LEFT_UP, true}},
    {{InputDrv::BTN_LEFT_DOWN, false},  {InputDrv::BTN_LEFT_DOWN, true}},
    {{InputDrv::BTN_RIGHT_UP, false},   {InputDrv::BTN_RIGHT_UP, true}},
    {{InputDrv::BTN_RIGHT_DOWN, false}, {InputDrv::BTN_RIGHT_DOWN, true}},
    {{InputDrv::BTN_USR, false},        {InputDrv::BTN_USR, true}}
};

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
InputDrv& InputDrv::GetInstance(void)
{
  // This class is static and declared here
  static InputDrv input_drv;
  // Return reference to class
  return input_drv;
}

// *****************************************************************************
// ***   Init Input Driver Task   **********************************************
// *****************************************************************************
Result InputDrv::InitTask(TIM_HandleTypeDef& htm, uint32_t ch)
{
  // Save timer handle and channel
  htim = &htm;
  channel = ch;
  // Create task
  return AppTask::InitTask();
}

// *****************************************************************************
// ***   Input Driver Setup   **************************************************
// *****************************************************************************
Result InputDrv::Setup()
{
  // Start timer to process encoder
  if(htim != nullptr)
  {
    HAL_TIM_Encoder_Start(htim, channel);
  }
  // Init ticks variable
  last_wake_ticks = RtosTick::GetTickCount();
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Input Driver Loop   ***************************************************
// *****************************************************************************
Result InputDrv::Loop()
{
  // Process all buttons
  for(uint32_t i = 0U; i < BTN_MAX; i++)
  {
    if(ProcessButtonInput(buttons[i]))
    {
      // Lock mutex before walking the callback list
      mutex.Lock();
      // Pointer to callback list element
      CallbackListEntry* btn_cbl = btn_callback_list;
      // Send notification
      while(btn_cbl != nullptr)
      {
        if(btn_cbl->mask & (1u << i))
        {
          // Copy callback list entry before releasing the mutex
          CallbackListEntry btn_cbl_data = *btn_cbl;
          // Release mutex before call callback: entry data is copied and
          // callback itself may add/remove handlers
          mutex.Release();

          // If there no AppTask pointer
          if(btn_cbl_data.callback_task == nullptr)
          {
            // Call callback directly in InputDrv task(semaphores in other task may be needed!)
            btn_cbl_data.callback(btn_cbl_data.obj_ptr, &btn_callback_data[i][buttons[i].btn_state]);
          }
          else
          {
            // Otherwise call it via AppTask to execute callback in target task
            btn_cbl_data.callback_task->Callback(btn_cbl_data.callback, btn_cbl_data.obj_ptr, &btn_callback_data[i][buttons[i].btn_state]);
          }
          // Lock mutex again before break the cycle
          mutex.Lock();
          // Only first handler gets notification
          break;
        }
        btn_cbl = btn_cbl->next;
      }
      // Release mutex after callback list is processed
      mutex.Release();
    }
  }

  // Process encoder
  int32_t enc_val = GetEncoderState(last_enc_value);
  // If it isn't 0
  if(enc_val)
  {
    uint32_t ms_per_click = (RtosTick::GetTimeMs() - last_enc_ms) / abs(enc_val);
    // Save timestamp
    last_enc_ms = RtosTick::GetTimeMs();
    // We limited 1 click per ms (or 10 revolution per second for 100 clicks encoder)
    if(ms_per_click == 0u)
    {
      ms_per_click = 1u;
    }
    // Calculate speed(clicks per second)
    enc_speed = 1000u / ms_per_click;

    // Lock mutex before walking the callback list
    mutex.Lock();
    // Pointer to callback list element
    CallbackListEntry* enc_cbl = enc_callback_list;
    // Send notification
    while(enc_cbl != nullptr)
    {
      // Copy callback list entry before releasing the mutex
      CallbackListEntry enc_cbl_data = *enc_cbl;
      // Release mutex before call callback: entry data is copied and
      // callback itself may add/remove handlers
      mutex.Release();

      // If there no AppTask pointer
      if(enc_cbl_data.callback_task == nullptr)
      {
        // Call callback directly in InputDrv task(semaphores in other task may be needed!)
        enc_cbl_data.callback(enc_cbl_data.obj_ptr, (void*)enc_val);
      }
      else
      {
        // Otherwise call it via AppTask to execute callback in target task
        enc_cbl_data.callback_task->Callback(enc_cbl_data.callback, enc_cbl_data.obj_ptr, (void*)enc_val);
      }
      // Lock mutex again before break the cycle
      mutex.Lock();
      // Only first handler gets notification
      break;
    }
    // Release mutex after callback list is processed
    mutex.Release();
  }

  // Pause until next tick
  RtosTick::DelayUntilMs(last_wake_ticks, 1U);
  // Always run
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Process Button Input function   ***************************************
// *****************************************************************************
bool InputDrv::ProcessButtonInput(ButtonProfile& button)
{
  // Flag to return if button state was changed
  bool is_changed = false;
  // Read Button state
  bool new_status = (HAL_GPIO_ReadPin(button.button_port, button.button_pin) == button.pin_state);

  // No sense do something if button status already set
  if(button.btn_state != new_status)
  {
    if((button.btn_state_tmp == new_status) && (button.btn_state_cnt == BUTTON_READ_DELAY))
    {
      // If temporary button state true and delay done - update state
      button.btn_state = new_status;
      // Button state changed
      is_changed = true;
    }
    else if(button.btn_state_tmp == new_status)
    {
      // If temporary button state true and delay not done - increase counter
      button.btn_state_cnt++;
    }
    else
    {
      // If temporary button state true and delay not done - increase counter
      button.btn_state_tmp = new_status;
      button.btn_state_cnt = 0;
    }
  }

  return is_changed;
}

// *****************************************************************************
// ***   Public: Add Buttons Callback handler   ********************************
// *****************************************************************************
void InputDrv::AddButtonsCallbackHandler(AppTask* callback_task, CallbackPtr callback, void* obj_ptr, uint8_t mask, CallbackListEntry& cble)
{
  // Set data in callback entry
  cble.callback_task = callback_task;
  cble.callback = callback;
  cble.obj_ptr = obj_ptr;
  cble.mask = mask;

  // Lock mutex
  mutex.Lock();

  // If callback list is empty
  if(btn_callback_list == nullptr)
  {
    // Clear pointers just in case
    cble.next = nullptr;
    cble.prev = nullptr;
    // Set as first element
    btn_callback_list = &cble;
  }
  else // Otherwise
  {
    // Try to find element in list
    CallbackListEntry* btn_cbl = btn_callback_list;
    // Try to find this handler in the list
    while(btn_cbl != nullptr)
    {
      if(btn_cbl == &cble) break;   // Handler found - break cycle
      else btn_cbl = btn_cbl->next; // Handler not found - set next element pointer
    }
    // Check if this handler already in the list and if not
    if(btn_cbl != &cble)
    {
      // Set as previous into existing head element
      btn_callback_list->prev = &cble;
      // Set next element pointer
      cble.next = btn_callback_list;
      // Clear previous pointers just in case
      cble.prev = nullptr;
      // Set as head
      btn_callback_list = &cble;
    }
  }

  // Release mutex
  mutex.Release();
}

// *****************************************************************************
// ***   Public: Delete Buttons Callback handler   *****************************
// *****************************************************************************
void InputDrv::DeleteButtonsCallbackHandler(CallbackListEntry& cble)
{
  // Lock mutex
  mutex.Lock();

  // If requested first element in list
  if(btn_callback_list == &cble)
  {
    btn_callback_list = cble.next;
    // List may become empty - check pointer before dereference
    if(btn_callback_list != nullptr) btn_callback_list->prev = nullptr;
  }
  else // Otherwise
  {
    // Try to find element in list
    CallbackListEntry* btn_cbl = btn_callback_list;
    // Try to find this handler in the list
    while(btn_cbl != nullptr)
    {
      if(btn_cbl == &cble) break;   // Handler found - break cycle
      else btn_cbl = btn_cbl->next; // Handler not found - set next element pointer
    }
    // Check if this handler in the list
    if(btn_cbl == &cble)
    {
      // Set next pointer for previous element
      cble.prev->next = cble.next;
      // Id next element exist
      if(cble.next != nullptr)
      {
        // Set it as next element for previous one
        cble.next->prev = cble.prev;
      }
    }
  }
  // Clear pointers in removed entry to prevent stale links on reuse
  cble.next = nullptr;
  cble.prev = nullptr;

  // Release mutex
  mutex.Release();
}

// *****************************************************************************
// ***   Public: Add Encoder Callback handler   ********************************
// *****************************************************************************
void InputDrv::AddEncoderCallbackHandler(AppTask* callback_task, CallbackPtr callback, void* obj_ptr, CallbackListEntry& cble)
{
  // Set data in callback entry
  cble.callback_task = callback_task;
  cble.callback = callback;
  cble.obj_ptr = obj_ptr;

  // Lock mutex
  mutex.Lock();

  // If callback list is empty
  if(enc_callback_list == nullptr)
  {
    // Clear pointers just in case
    cble.next = nullptr;
    cble.prev = nullptr;
    // Set as first element
    enc_callback_list = &cble;
  }
  else // Otherwise
  {
    // Try to find element in list
    CallbackListEntry* btn_cbl = enc_callback_list;
    // Try to find this handler in the list
    while(btn_cbl != nullptr)
    {
      if(btn_cbl == &cble) break;   // Handler found - break cycle
      else btn_cbl = btn_cbl->next; // Handler not found - set next element pointer
    }
    // Check if this handler already in the list and if not
    if(btn_cbl != &cble)
    {
      // Set as previous into existing head element
      enc_callback_list->prev = &cble;
      // Set next element pointer
      cble.next = enc_callback_list;
      // Clear previous pointers just in case
      cble.prev = nullptr;
      // Set as head
      enc_callback_list = &cble;
    }
  }

  // Release mutex
  mutex.Release();
}

// *****************************************************************************
// ***   Public: Delete Encoder Callback handler   *****************************
// *****************************************************************************
void InputDrv::DeleteEncoderCallbackHandler(CallbackListEntry& cble)
{
  // Lock mutex
  mutex.Lock();

  // If requested first element in list
  if(enc_callback_list == &cble)
  {
    enc_callback_list = cble.next;
    // List may become empty - check pointer before dereference
    if(enc_callback_list) enc_callback_list->prev = nullptr;
  }
  else // Otherwise
  {
    // Try to find element in list
    CallbackListEntry* btn_cbl = enc_callback_list;
    // Try to find this handler in the list
    while(btn_cbl != nullptr)
    {
      if(btn_cbl == &cble) break;   // Handler found - break cycle
      else btn_cbl = btn_cbl->next; // Handler not found - set next element pointer
    }
    // Check if this handler in the list
    if(btn_cbl == &cble)
    {
      // Set next pointer for previous element
      cble.prev->next = cble.next;
      // Id next element exist
      if(cble.next != nullptr)
      {
        // Set it as next element for previous one
        cble.next->prev = cble.prev;
      }
    }
  }
  // Clear pointers in removed entry to prevent stale links on reuse
  cble.next = nullptr;
  cble.prev = nullptr;

  // Release mutex
  mutex.Release();
}

// *****************************************************************************
// ***   Get button current state   ********************************************
// *****************************************************************************
bool InputDrv::GetButtonCurrentState(ButtonType button)
{
  // Return current state of button
  return buttons[button].btn_state;
}

// *****************************************************************************
// ***   Get button state   ****************************************************
// *****************************************************************************
bool InputDrv::GetButtonState(ButtonType button)
{
  // Return current state of button
  return GetButtonState(button, buttons_last_state[button]);
}

// *****************************************************************************
// ***   Get button state   ****************************************************
// *****************************************************************************
bool InputDrv::GetButtonState(ButtonType button, bool& btn_state)
{
  bool ret = false;
  // If button state changed
  if(buttons[button].btn_state != btn_state)
  {
    // Store new state
    btn_state = buttons[button].btn_state;
    ret = true;
  }
  // Return result
  return ret;
}

// *****************************************************************************
// ***   Get encoder counts from last call   ***********************************
// *****************************************************************************
int32_t InputDrv::GetEncoderState(int32_t& last_enc_val)
{
  // Get current state - atomic operation, prevent multitasking problems
  int32_t enc_val = htim->Instance->CNT;
  // Calculate difference on raw counts using 16-bit unsigned wraparound
  // subtraction: correct for a counter with 16-bit period and for a 32-bit
  // counter as well, since the difference between polls is always small.
  // Divide by 2 after that to convert counts to clicks.
  int32_t retval = (int32_t)((int16_t)((uint16_t)enc_val - (uint16_t)last_enc_val)) / 2;
  // Advance saved value only by consumed clicks to keep half-click remainder
  last_enc_val += retval * 2;
  // return result
  return retval;
}

