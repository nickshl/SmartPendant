//******************************************************************************
//  @file UiButton.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: UI Button Visual Object Class, header
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

#ifndef UiButton_h
#define UiButton_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"
#include "VisObject.h"

// *****************************************************************************
// ***   Button Class   ********************************************************
// *****************************************************************************
class UiButton : public VisObject
{
  public:
    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    UiButton() {};

    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    UiButton(const char* str_in, int32_t x, int32_t y, int32_t w, int32_t h,
             bool is_active = false);

    // *************************************************************************
    // ***   SetParams   *******************************************************
    // *************************************************************************
    void SetParams(const char* str_in, int32_t x, int32_t y, int32_t w, int32_t h,
                   bool is_active = false);

    // *************************************************************************
    // ***   SetString   *******************************************************
    // *************************************************************************
    void SetString(const char* str_in);

    // *************************************************************************
    // ***   Public: SetColor   ************************************************
    // *************************************************************************
    void SetColor(color_t c);

    // *************************************************************************
    // ***   Public: SetFont   *************************************************
    // *************************************************************************
    void SetFont(Font& font);

    // *************************************************************************
    // ***   Public: SetScale   ************************************************
    // *************************************************************************
    void SetScale(uint8_t scale);

    // *************************************************************************
    // ***   Enable   **********************************************************
    // *************************************************************************
    void Enable();

    // *************************************************************************
    // ***   Disable   *********************************************************
    // *************************************************************************
    void Disable();

    // *************************************************************************
    // ***   SetPressed   ******************************************************
    // *************************************************************************
    void SetPressed(bool pressed);

    // *************************************************************************
    // ***   GetPressed   ******************************************************
    // *************************************************************************
    bool GetPressed(void) {return is_pressed;}

    // *************************************************************************
    // ***   Set callback function   *******************************************
    // *************************************************************************
    void SetCallback(AppTask* task, CallbackPtr func = nullptr, void* param = nullptr);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t y = 0);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t x = 0);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void Action(VisObject::ActionType action, int32_t tx, int32_t ty, int32_t tpx, int32_t tpy);

  private:
    // Callback function pointer
    AppTask* callback_task = nullptr;
    CallbackPtr callback_func = nullptr;
    void* callback_param = nullptr;

    // String pointer
    const char* str = nullptr;
    // String color
    color_t color = COLOR_WHITE;
    // Flag to draw button pressed always
    bool is_pressed = false;
    // Flag to draw button pressed only when it touched
    bool draw_pressed = false;
    // String for button
    String string;
    // String for shadow
    String string_shadow;
};

#endif // UiButton_h
