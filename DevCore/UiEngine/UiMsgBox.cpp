//******************************************************************************
//  @file UiEngine.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: UI Message Box Class, implementation
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
#include "UiEngine.h"

// *****************************************************************************
// ***   Public: MsgBox constructor   ******************************************
// *****************************************************************************
UiMsgBox::UiMsgBox(const char* msg_in, const char* hdr_in,
                   Font* msg_fnt_in, Font* hdr_fnt_in,
                   uint16_t center_x_in, uint16_t center_y_in,
                   uint16_t width_in, color_t color_in)
{
  // Save input params
  msg = msg_in;            // Message
  msg_fnt = msg_fnt_in;    // Message font
  hdr = hdr_in;            // Header string
  hdr_fnt = hdr_fnt_in;    // Header font
  center_x = center_x_in;  // X position of MsgBox center
  center_y = center_y_in;  // Y position of MsgBox center
  width = width_in;        // MsgBox width in characters
  color = color_in;        // Color

  // Variables for store window dimension
  int16_t X = 0, Y = 0, W = 0, H = 0, StrW = 0;
  // Pointers to string array
  char* line[MAX_MSGBOX_LINES];
  // Strings length array
  uint8_t length[MAX_MSGBOX_LINES];
  // Strings count
  uint8_t count = 0;

  // Pointer to message should present
  if(msg != nullptr)
  {
    // Set fonts by default if isn't provided
    if(msg_fnt == nullptr) msg_fnt = &Font_8x12::GetInstance();
    if(hdr_fnt == nullptr) hdr_fnt = &Font_4x6::GetInstance();

    // Copy string to buffer for split
    strcpy(str_buf, msg);

    // Set first string to buffer
    line[count] = str_buf;

    // Find MsgBox width in pixels
    W = msg_fnt->GetCharW() * width;
  
    // Split buffer to strings
    while(count < MAX_MSGBOX_LINES)
    {
      // Search pointer to '\n' symbol
      char* ptr = strchr(line[count], (int)'\n');

      // If not found
      if(ptr == nullptr)
      {
        // Set string length with strlen()
        length[count] = strlen(line[count]);
      }
      else
      {
        // Calculate string length by pointers difference
        length[count] = ptr - line[count];
        // Replace symbol '\n' to '\0' for terminate string
        *ptr = '\0';
      }

      // Calculate string width in pixels
      StrW = msg_fnt->GetCharW() * length[count];
      // If this string width greater than previous - store it
      if(W < StrW) W = StrW;
      // Add sting height
      H += msg_fnt->GetCharH();

      // If symbol '\n' isn't found 0 this is last string - exit from the cycle
      if(ptr == nullptr) break;

      // Increment string counter
      count++;
      // Set pointer to the next string
      line[count] = ptr + 1;
    }

    if(hdr != nullptr)
    {
      // Calculate header width in pixels
      StrW = hdr_fnt->GetCharW() * strlen(hdr);
      // If header width is greater than string width - store it
      if(W < StrW) W = StrW;
    }

    // Add left and right empty space 1 symbol width
    W += msg_fnt->GetCharW() * 2;
    // Add up and down empty space 0.5 symbol width
    H += msg_fnt->GetCharH();

    // Calculate window position
    X = center_x - W / 2;
    Y = center_y - H / 2;

    // Header if present
    if(hdr != nullptr)
    {
      // Move window down
      Y += hdr_fnt->GetCharH() / 2;
      // Header border
      box[box_cnt++].SetParams(X - 1, Y - hdr_fnt->GetCharH() - 2, W + 1, hdr_fnt->GetCharH() + 3, COLOR_MAGENTA, false);
      // Header place
      box[box_cnt++].SetParams(X, Y - hdr_fnt->GetCharH() - 1, W, hdr_fnt->GetCharH() + 1, COLOR_MAGENTA, true);
      // Header string
      string[str_cnt++].SetParams(hdr, X + 1, Y - hdr_fnt->GetCharH(), COLOR_YELLOW, *hdr_fnt);
    }

    // Message place
    box[box_cnt++].SetParams(X - 1, Y - 1, W + 2, H + 2, COLOR_BLACK, false);
    // Message border
    box[box_cnt++].SetParams(X, Y, W, H, COLOR_MAGENTA, false);

    // Set result strings
    for(uint8_t i = 0; i < count + 1; i++)
    {
      // Calculate sting X position
      X = center_x - (msg_fnt->GetCharW() * length[i]) / 2;
      // Set string params
      string[str_cnt++].SetParams(line[i], X, Y + msg_fnt->GetCharH()/2 + i * msg_fnt->GetCharH(), COLOR_YELLOW, *msg_fnt);
    }
  }
}

// *************************************************************************
// ***   Public: Destructor   **********************************************
// *************************************************************************
UiMsgBox::~UiMsgBox()
{
  // Hide before destruct for memory clean up
  Hide();
}

// *****************************************************************************
// ***   Show MsgBox   *********************************************************
// *****************************************************************************
void UiMsgBox::Show(uint32_t z)
{
  for(uint32_t i = 0; i < box_cnt; i++)
  {
    box[i].Show(z);
  }
  for(uint32_t i = 0; i < str_cnt; i++)
  {
    string[i].Show(z + 1U);
  }
}

// *****************************************************************************
// ***   Hide MsgBox   *********************************************************
// *****************************************************************************
void UiMsgBox::Hide(void)
{
  // Delete boxes
  for(uint32_t i = 0; i < box_cnt; i++)
  {
    box[i].Hide();
  }
  // Delete strings
  for(uint32_t i = 0; i < str_cnt; i++)
  {
    string[i].Hide();
  }
}

// *****************************************************************************
// ***   Show and Hide Msg box after pause   ***********************************
// *****************************************************************************
void UiMsgBox::Run(uint32_t delay)
{
  Show();
  DisplayDrv::GetInstance().UpdateDisplay();
  RtosTick::DelayMs(delay);
  Hide();
}
