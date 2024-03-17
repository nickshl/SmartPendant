//******************************************************************************
//  @file UiMenu.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: UI Menu Class, implementation
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
#include "UiMenu.h"

// *****************************************************************************
// ***   Public: Menu constructor   ********************************************
// *****************************************************************************
UiMenu::UiMenu(const char* header_str_in, MenuItem* items_in, int32_t items_cnt_in,
               int32_t current_pos_in, Font* header_font_in, Font* items_font_in,
               int16_t x, int16_t y, int16_t w, int16_t h)
{
  // Save input params
  header_str = header_str_in;   // Header string
  items = items_in;             // Items array
  items_cnt = items_cnt_in;     // Items count
  current_pos = current_pos_in; // Current position
  header_font = header_font_in; // Header font
  items_font = items_font_in;   // Items font
  x_start = x;                  // X position
  y_start = y;                  // Y position
  // If width is 0 - get width of all screen
  width = w ? w : DisplayDrv::GetInstance().GetScreenW();  // Menu width
  // If height is 0 - get width of all screen
  height = h ? h : DisplayDrv::GetInstance().GetScreenH(); // Menu height
  // Set fonts
  header_font = header_font_in;
  items_font = items_font_in;
  // Set default fonts if needed
  if(header_font == nullptr) header_font = &Font_10x18::GetInstance();
  if(items_font == nullptr) items_font = &Font_8x12::GetInstance();
}

// *****************************************************************************
// ***   Public: Show   ********************************************************
// *****************************************************************************
void UiMenu::Show(void)
{
  // Checks
  if(items == nullptr) return;
  if(items_cnt == 0) return;

  // Get header string height
  if(header_str != nullptr) header_height = header_font->GetCharH();
  // Menu items count fit on the screen
  menu_count = (height - header_height - 3) / items_font->GetCharH();
  // If screen fit more items than we have - limit it
  if(menu_count > items_cnt) menu_count = items_cnt;
  if(menu_count > (int16_t)MAX_MENU_ITEMS) menu_count = MAX_MENU_ITEMS;

  // Height of the Scroll bar: heigth of the Screen - height of the Header
  int16_t scroll_h = height - (header_height + 2);
  // Width of the Scroll bar: same as Items font width
  int16_t scroll_w = items_font->GetCharW() - 2;
  // Count of characters in the line plus null-terminator
  str_len = (width - (scroll_w + 2)) / items_font->GetCharW() + 1;
 
  // Menu border
  box.SetParams(x_start - 1, x_start - 1, width + 2, height + 2, COLOR_GREEN, false);
  // Show menu border
  box.Show(100);

  // Create header String object
  hdr_str.SetParams(header_str, x_start + items_font->GetCharW(), y_start, COLOR_YELLOW, *header_font);
  // Create header Line object
  hdr_line.SetParams(x_start, y_start + header_height, width, header_height, COLOR_MAGENTA);
  // If have caption string
  if(header_str != nullptr)
  {
    hdr_str.Show(100);  // Show caption string
    hdr_line.Show(100); // Show caption line
  }

  // Create scroll for menu
  scroll.SetParams(x_start + 1, y_start + height - scroll_h - 1, scroll_w, scroll_h, items_cnt, menu_count, true, false);
  // Set position
  scroll.SetScrollPos(current_pos);
  // Show scroll
  scroll.Show(1001);

  // Box for selected item
  selection_bar.SetParams(x_start + items_font->GetCharW(), y_start + items_font->GetCharH() + header_height + 2,
                          width - items_font->GetCharW() - 1, items_font->GetCharH() - 1, COLOR_RED, true);
  // Show selection bar
  selection_bar.Show(100);

  // Allocate memory for menu items
  for(uint32_t i = 0; i < MAX_MENU_ITEMS; i++)
  {
    // Create char array for string
    menu_txt[i] = new char[str_len];
    // Clear string - add null-terminator in the first position
    menu_txt[i][0] = '\0';
    // Create string for menu item
    menu_str[i] = new String(menu_txt[i], x_start + items_font->GetCharW(),
                             y_start + items_font->GetCharH() * i + header_height + 2,
                             COLOR_CYAN, *items_font);
    // Show string
    menu_str[i]->Show(101);
  }
}

// *****************************************************************************
// ***   Public: Hide   ********************************************************
// *****************************************************************************
void UiMenu::Hide(void)
{
  // Clear memory
  for(uint32_t i = 0; i < MAX_MENU_ITEMS; i++)
  {
    delete(menu_str[i]);
    delete(menu_txt[i]);
  }

  // Hide all objects
  box.Hide();
  hdr_str.Hide();
  hdr_line.Hide();
  selection_bar.Hide();
  scroll.Hide();
}

// *****************************************************************************
// ***   Public: Main Menu cycle   *********************************************
// *****************************************************************************
bool UiMenu::Run(void)
{
  bool ret = false;

  int16_t start_pos = 0;

  char tmp_str[64];

  // Show menu
  Show();

  // Init user input before run Menu cycle
  InitUserInput();

  do
  {
    do
    {
      // Lock display because we will change strings content
      DisplayDrv::GetInstance().LockDisplay();
      // Draw menu items
      for(int32_t i = 0; i < menu_count; i++)
      {
        // Fill temporary buffer with spaces
        memset(menu_txt[i], ' ', str_len);

        // If we have function for additional string generate
        if(items[i+start_pos].GetStr != nullptr)
        {
          // Get additional string in the buffer
          items[i+start_pos].GetStr(items[current_pos].ptr, tmp_str, sizeof(tmp_str), items[i+start_pos].add_param);
          // Get additional string size
          uint32_t buf_len = strlen(tmp_str);
          // Limit string length
          if(buf_len >= (uint32_t)str_len) buf_len = str_len - 1;
          // Copy data with right-alligment
          strncpy(menu_txt[i] + (str_len - buf_len - 1), tmp_str, buf_len - 1);
        }

        // Copy item to the screen array
        snprintf(menu_txt[i], str_len, "%s", items[i+start_pos].str);
        // Remove null-terminator for stick two strings
        if(strlen(menu_txt[i]) < (size_t)str_len) menu_txt[i][strlen(menu_txt[i])] = ' ';
        // Set null-terminator at the end
        menu_txt[i][str_len] = '\0';
      }
      // Move selection bar
      selection_bar.Move(x_start + items_font->GetCharW(),
                         y_start + items_font->GetCharH() * (current_pos - start_pos) + header_height + 2);
      // Unlock display
      display_drv.UnlockDisplay();
      // Refresh display
      display_drv.UpdateDisplay();
      // Delay for update screen
      RtosTick::DelayTicks(100);

      // Process user input
      ProcessUserInput();

      // If value the same - scroll wasn't touched
      if(scroll.GetScrollPos() == current_pos)
      {
        // Change cursor position if user press UP or DOWN
        if((kbd_up)   && (current_pos > 0))             current_pos--;
        if((kbd_down) && (current_pos < items_cnt - 1)) current_pos++;
        // Update scroll value
        scroll.SetScrollPos(current_pos);
      }
      else
      {
        // Update menu position from scroll value
        current_pos = scroll.GetScrollPos();
      }

      // If cursor out of screen - scroll menu
      // "while" used instead "if" because position can be changed more than 1
      // if scroll will be touched
      while(current_pos < start_pos) start_pos--;
      while(current_pos > start_pos + menu_count - 1) start_pos++;
    }
    while(!(kbd_right || kbd_left));

    // If user pressed ENTER
    if(kbd_right)
    {
      // And we have callback function
      if(items[current_pos].Callback != nullptr)
      {
        // Call it
        items[current_pos].Callback(items[current_pos].ptr, items[current_pos].add_param);
      }
      else // Otherwise 
      {
        // Set return flag
        ret = true;
        // And break cycle to exit
        break;
      }
    }
  }
  while(!(kbd_left));

  // Hide menu
  Hide();

  // Return result
  return ret;
}

// *****************************************************************************
// ***   Private: Init user input   ********************************************
// *****************************************************************************
void UiMenu::InitUserInput(void)
{
#if defined(INPUTDRV_ENABLED)
  // Init last buttons values
  (void) input_drv.GetButtonState(InputDrv::EXT_LEFT, InputDrv::BTN_UP, up_btn_val);
  (void) input_drv.GetButtonState(InputDrv::EXT_LEFT, InputDrv::BTN_RIGHT, right_btn_val);
  (void) input_drv.GetButtonState(InputDrv::EXT_LEFT, InputDrv::BTN_DOWN, down_btn_val);
  (void) input_drv.GetButtonState(InputDrv::EXT_LEFT, InputDrv::BTN_LEFT, left_btn_val);

  // Init last encoders & buttons values
  (void) input_drv.GetEncoderState(InputDrv::EXT_LEFT,  last_enc_left_val);
  (void) input_drv.GetEncoderState(InputDrv::EXT_RIGHT, last_enc_right_val);
  (void) input_drv.GetEncoderButtonState(InputDrv::EXT_LEFT,  InputDrv::ENC_BTN_ENT, enc1_btn_left_val);
  (void) input_drv.GetEncoderButtonState(InputDrv::EXT_RIGHT, InputDrv::ENC_BTN_ENT, enc2_btn_left_val);
#endif
}

// *****************************************************************************
// ***   Private: Process user input   *****************************************
// *****************************************************************************
void UiMenu::ProcessUserInput(void)
{
#if defined(INPUTDRV_ENABLED)
  // Variable will be set to true if key status changed and new status
  // empty - only when key will be released
  if(   (input_drv.GetDeviceType(InputDrv::EXT_LEFT) == InputDrv::EXT_DEV_BTN)
     || (input_drv.GetDeviceType(InputDrv::EXT_LEFT) == InputDrv::EXT_DEV_JOY) )
  {
    kbd_up = input_drv.GetButtonState(InputDrv::EXT_LEFT, InputDrv::BTN_UP, up_btn_val) && !up_btn_val;
    kbd_right = input_drv.GetButtonState(InputDrv::EXT_LEFT, InputDrv::BTN_RIGHT, right_btn_val) && !right_btn_val;
    kbd_down = input_drv.GetButtonState(InputDrv::EXT_LEFT, InputDrv::BTN_DOWN, down_btn_val) && !down_btn_val;
    kbd_left = input_drv.GetButtonState(InputDrv::EXT_LEFT, InputDrv::BTN_LEFT, left_btn_val) && !left_btn_val;
  }
  // Handle left encoder
  if(input_drv.GetDeviceType(InputDrv::EXT_LEFT) == InputDrv::EXT_DEV_ENC)
  {
    kbd_left = input_drv.GetEncoderButtonState(InputDrv::EXT_LEFT, InputDrv::ENC_BTN_BACK, left_btn_val) && !left_btn_val;
    kbd_right = input_drv.GetEncoderButtonState(InputDrv::EXT_LEFT, InputDrv::ENC_BTN_ENT, right_btn_val) && !right_btn_val;
    int32_t enc_val = input_drv.GetEncoderState(InputDrv::EXT_LEFT, last_enc_left_val);
    kbd_up = (enc_val < 0);
    kbd_down = (enc_val > 0);
  }
#endif
}
