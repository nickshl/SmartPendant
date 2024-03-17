//******************************************************************************
//  @file UiScroll.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: UI Scroll Visual Object Class, header
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

#ifndef UiScroll_h
#define UiScroll_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "DisplayDrv.h"
#include "VisObject.h"

// *****************************************************************************
// ***   Scroll Class   ********************************************************
// *****************************************************************************
class UiScroll : public VisObject
{
  public:
    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    UiScroll() {};

    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    UiScroll(int32_t x, int32_t y, int32_t w, int32_t h, int32_t n, int32_t bar,
             bool is_vertical = true, bool is_has_buttons = false, bool is_active = true);

    // *************************************************************************
    // ***   SetParams   *******************************************************
    // *************************************************************************
    void SetParams(int32_t x, int32_t y, int32_t w, int32_t h, int32_t n,
                   int32_t bar, bool is_vertical = true,
                   bool is_has_buttons = false, bool is_active = true);

    // *************************************************************************
    // ***   Return End X coordinate   *****************************************
    // *************************************************************************
    int32_t GetScrollPos(void) {return cnt;};

    // *************************************************************************
    // ***   Return End Y coordinate   *****************************************
    // *************************************************************************
    void SetScrollPos(int32_t pos) {cnt = pos;};

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
    // Current position
    int32_t cnt = 0;
    // Total count
    int32_t total_cnt = 0;
    // number of records "shows" on the screen for properly scale bar
    int32_t bar_cnt = 0;
    // Is scroll vertical?
    bool vertical = false;
    // Is has buttons
    bool has_buttons = false;
    
    // Calculated data
    int16_t bar_shift = 0; // Bar initial shift for skip button place
    int16_t total_len = 0; // Total length for bar
    int16_t bar_len = 0;   // Bar length
    int16_t empty_len = 0; // Empty length available for bar moving
};

#endif // UiScroll_h
