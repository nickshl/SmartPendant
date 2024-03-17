//******************************************************************************
//  @file UiMenu.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: UI Menu Class, header
//
//  @section LICENSE
//
//   Software License Agreement (Modified BSD License)
//
//   Copyright (c) 2016, Devtronic & Nicolai Shlapunov
//   All rights reserved.
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//   3. Neither the name of the Devtronic nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//   4. Redistribution and use of this software other than as permitted under
//      this license is void and will automatically terminate your rights under
//      this license.
//
//   THIS SOFTWARE IS PROVIDED BY DEVTRONIC ''AS IS'' AND ANY EXPRESS OR IMPLIED
//   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL DEVTRONIC BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
//   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  @section SUPPORT
//
//   Devtronic invests time and resources providing this open source code,
//   please support Devtronic and open-source hardware/software by
//   donations and/or purchasing products from Devtronic.
//
//******************************************************************************

#ifndef UiMenu_h
#define UiMenu_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"
#include "InputDrv.h"
#include "SoundDrv.h"
#include "UiEngine.h"

// *****************************************************************************
// ***   Menu Class   **********************************************************
// *****************************************************************************
class UiMenu
{
  public:
    // *************************************************************************
    // ***   Menu Item description structure   *********************************
    // *************************************************************************
    typedef struct typeMenuItem
    {
        const char* str;                             // Menu Item caption
        void (*Callback)(void* ptr, uint32_t param); // Callback for menu item enter
        char* (*GetStr) (void* ptr, char* buf, uint32_t n, uint32_t add_param); // Callback for string generation
        void* ptr;                             // Pointer for callbacks(pointer to object)
        uint32_t add_param;                    // Additional params for callbacks
    } MenuItem;

    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    UiMenu(const char* header_str_in, MenuItem* items_in, int32_t items_cnt_in,
           int32_t current_pos_in = 0,
           Font* header_font_in = nullptr, Font* items_font_in = nullptr,
           int16_t x = 0, int16_t y = 0, int16_t w = 0, int16_t h = 0);

    // *************************************************************************
    // ***   Public: Show   ****************************************************
    // *************************************************************************
    void Show(void);

    // *************************************************************************
    // ***   Public: Hide   ****************************************************
    // *************************************************************************
    void Hide(void);

    // *************************************************************************
    // ***   Public: Run menu   ************************************************
    // *************************************************************************
    bool Run(void);

    // *************************************************************************
    // ***   Public: GetCurrentPosition   **************************************
    // *************************************************************************
    inline int32_t GetCurrentPosition(void) {return current_pos;}

  private:
    // Max allowed menu items on the screen
    static const uint32_t MAX_MENU_ITEMS = 16U;

    const char* header_str; // Menu header
    Font* header_font;      // Header font
    MenuItem* items;        // Pointer to Items array
    int32_t items_cnt;      // Items count
    int32_t current_pos;    // Current position
    Font* items_font;       // Menu items font
    int16_t x_start;
    int16_t y_start;
    int16_t width;
    int16_t height;

    int16_t header_height = 0;
    int16_t menu_count = 0;
    int16_t str_len = 0;

    // Strings for menu items
    String* menu_str[MAX_MENU_ITEMS];
    char* menu_txt[MAX_MENU_ITEMS];

    // Box across menu
    Box box;
    // Header String object
    String hdr_str;
    // Header line
    Line hdr_line;
    // Box for selected item
    Box selection_bar;
    // Scroll
    UiScroll scroll;

    // Variables for user input
    bool kbd_up = false;
    bool kbd_right = false;
    bool kbd_down = false;
    bool kbd_left = false;
    // Last buttons values
    bool up_btn_val = false;
    bool right_btn_val = false;
    bool down_btn_val = false;
    bool left_btn_val = false;
    // Last encoders & buttons values
    int32_t last_enc_left_val = 0;
    int32_t last_enc_right_val = 0;
    bool enc1_btn_left_val = false;
    bool enc2_btn_left_val = false;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();

#if defined(INPUTDRV_ENABLED)
    // Input driver instance
    InputDrv& input_drv = InputDrv::GetInstance();
#endif
#if defined(SOUNDDRV_ENABLED)
    // Sound driver instance
    SoundDrv& sound_drv = SoundDrv::GetInstance();
#endif

    // *************************************************************************
    // ***   Private: Init user input   ****************************************
    // *************************************************************************
    void InitUserInput(void);

    // *************************************************************************
    // ***   Private: Process user input   *************************************
    // *************************************************************************
    void ProcessUserInput(void);
};

#endif // UiMenu_h
