//******************************************************************************
//  @file MultiLineString.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: MultiLineString Visual Object Class, header
//
//  @section LICENSE
//
//   Software License Agreement (BSD License)
//
//   Copyright (c) 2016, Devtronic & Nicolai Shlapunov
//   All rights reserved.
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are met:
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//   3. Neither the name of the Devtronic nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
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
//******************************************************************************

#ifndef MultiLineString_h
#define MultiLineString_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "IDisplay.h"
#include "VisObject.h"
#include "Font.h"
#include "Font_4x6.h"
#include "Font_6x8.h"
#include "Font_8x8.h"
#include "Font_8x12.h"
#include "Font_10x18.h"
#include "Font_12x16.h"

// *****************************************************************************
// ***   MultiLineString Class   ***********************************************
// *****************************************************************************
class MultiLineString : public VisObject
{
  public:
    // *************************************************************************
    // ***   Align Enum   ******************************************************
    // *************************************************************************
    typedef enum : uint8_t
    {
      LEFT = 0,
      CENTER,
      RIGHT,
      ALIGN_CNT
    } alignment_t;

    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    MultiLineString() {};
 
    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    MultiLineString(const char* str, int32_t x, int32_t y, color_t tc, Font& font);

    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    MultiLineString(const char* str, int32_t x, int32_t y, color_t tc, color_t bgc, Font& font);

    // *************************************************************************
    // ***   Public: SetParams   ***********************************************
    // *************************************************************************
    void SetParams(const char* str, int32_t x, int32_t y, color_t tc, Font& font);

    // *************************************************************************
    // ***   Public: SetParams   ***********************************************
    // *************************************************************************
    void SetParams(const char* str, int32_t x, int32_t y, color_t tc, color_t bgc, Font& font);

    // *************************************************************************
    // ***   Public: SetString   ***********************************************
    // *************************************************************************
    void SetString(const char* str, bool force = false);

    // *************************************************************************
    // ***   Public: GetString   ***********************************************
    // *************************************************************************
    const char* GetString() {return string;}

    // *************************************************************************
    // ***   Public: SetString   ***********************************************
    // *************************************************************************
    void SetString(char* buffer, uint32_t n, const char* format, ...);

    // *************************************************************************
    // ***   Public: SetColor   ************************************************
    // *************************************************************************
    void SetColor(color_t tc, color_t bgc = 0U, bool is_trnsp = true);

    // *************************************************************************
    // ***   Public: SetFont   *************************************************
    // *************************************************************************
    void SetFont(Font& font);

    // *************************************************************************
    // ***   Public: SetScale   ************************************************
    // *************************************************************************
    void SetScale(uint8_t s);

    // *************************************************************************
    // ***   Public: GetScale   ************************************************
    // *************************************************************************
    uint8_t GetScale(void) {return scale;}

    // *************************************************************************
    // ***   Public: SetSpacing   **********************************************
    // *************************************************************************
    void SetSpacing(uint8_t s);

    // *************************************************************************
    // ***   Public: GetSpacinge   *********************************************
    // *************************************************************************
    uint8_t GetSpacinge(void) {return spacing;}

    // *************************************************************************
    // ***   Public: SetAlignment   ********************************************
    // *************************************************************************
    void SetAlignment(alignment_t a);

    // *************************************************************************
    // ***   Public: Put line in buffer   **************************************
    // *************************************************************************
    virtual void DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t y = 0);

    // *************************************************************************
    // ***   Public: Put line in buffer   **************************************
    // *************************************************************************
    virtual void DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t x = 0);

    // *************************************************************************
    // ***   Public: GetFontW   ************************************************
    // *************************************************************************
    uint32_t GetFontW() {return ((font_ptr == nullptr) ? 0U : font_ptr->GetCharW());}

    // *************************************************************************
    // ***   Public: GetFontH   ************************************************
    // *************************************************************************
    uint32_t GetFontH() {return ((font_ptr == nullptr) ? 0U : font_ptr->GetCharH());}

    // *************************************************************************
    // ***   Public: GetFontBytePerChar   **************************************
    // *************************************************************************
    uint32_t GetFontBytePerChar() {return ((font_ptr == nullptr) ? 0U : font_ptr->GetBytesPerChar());}

  private:
    // Pointer to string
    const char* string = nullptr;
    // Text color
    color_t txt_color = 0u;
    // Background color
    color_t bg_color = 0u;
    // Scale
    uint8_t scale = 1u;
    // Font type
    Font* font_ptr = nullptr;
    // Is background transparent ?
    bool transpatent_bg = false;
    // Left alignment
    alignment_t alignment = LEFT;
    // Scale
    uint8_t spacing = 0u;

    // Line height: character height multiplied by scale and added spacing
    uint8_t line_height = 1u;
    // String line number
    int32_t str_line = -1;
    // String pointer
    const char* str_ptr;
    // Line length
    uint16_t str_len = 0u;

    // *************************************************************************
    // ***   Private: GetStringCount   *****************************************
    // *************************************************************************
    uint32_t GetStringCount(const char* str);

    // *************************************************************************
    // ***   Private: GetStringLength   ****************************************
    // *************************************************************************
    uint32_t GetStringLength(const char* str);

    // *************************************************************************
    // ***   Private: GetLongestLineLength   ***********************************
    // *************************************************************************
    uint32_t GetLongestLineLength(const char* str);
};

#endif
