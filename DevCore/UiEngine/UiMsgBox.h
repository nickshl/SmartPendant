//******************************************************************************
//  @file UiMsgBox.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: UI Message Box Class, header
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

#ifndef UiBox_h
#define UiBox_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"
#include "InputDrv.h"
#include "SoundDrv.h"
#include "UiEngine.h"
#include "Font.h"

// *****************************************************************************
// ***   Menu Class   **********************************************************
// *****************************************************************************
class UiMsgBox
{
  public:
    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    UiMsgBox(const char* msg_in, const char* hdr_in,
             Font* msg_fnt_in = nullptr, Font* hdr_fnt_in = nullptr,
             uint16_t center_x_in = DisplayDrv::GetInstance().GetScreenW()/2,
             uint16_t center_y_in = DisplayDrv::GetInstance().GetScreenH()/2,
             uint16_t width_in = 0, color_t color_in = 0);

    // *************************************************************************
    // ***   Public: Destructor   **********************************************
    // *************************************************************************
    ~UiMsgBox();

    // *************************************************************************
    // ***   Public: Show MsgBox   *********************************************
    // *************************************************************************
    void Show(uint32_t z = 0xFFFFFFF0);

    // *************************************************************************
    // ***   Public: Hide MsgBox   *********************************************
    // *************************************************************************
    void Hide(void);

    // *************************************************************************
    // ***   Public: Run MsgBox   **********************************************
    // *************************************************************************
    void Run(uint32_t delay);

  private:
    // Max allowed menu items on the screen
    static const uint32_t MAX_MSGBOX_LINES = 5U;

    // Pointer to message
    const char* msg;
    // Message font
    Font* msg_fnt;

    // Pointer to header
    const char* hdr;
    // Header font
    Font* hdr_fnt;

    // Position of MsgBox
    uint16_t center_x;
    uint16_t center_y;
    // Width of MsgBox
    uint16_t width;
    // Color of MsgBox
    color_t color;

    // Data
    Box box[4];
    uint16_t box_cnt = 0;
    String string[MAX_MSGBOX_LINES + 1U];
    uint16_t str_cnt = 0;

    // Buffer for stings
    char str_buf[128];
};

#endif // UiEngine_h
