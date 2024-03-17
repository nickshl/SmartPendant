//******************************************************************************
//  @file Font.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: Font interface, header
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

#ifndef Font_h
#define Font_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"

// *****************************************************************************
// ***   Font Class   **********************************************************
// *****************************************************************************
class Font
{
  public:
    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    explicit Font() {};

    // *************************************************************************
    // ***   Public: Destructor   **********************************************
    // *************************************************************************
    virtual ~Font() {};

    // *************************************************************************
    // ***   GetCharW   ********************************************************
    // *************************************************************************
    virtual uint32_t GetCharW() {return char_width;}

    // *************************************************************************
    // ***   GetCharH   ********************************************************
    // *************************************************************************
    virtual uint32_t GetCharH() {return char_height;}

    // *************************************************************************
    // ***   GetBytesPerChar   *************************************************
    // *************************************************************************
    virtual uint32_t GetBytesPerChar() {return bytes_per_char;}

    // *************************************************************************
    // ***   GetCharGataPtr   **************************************************
    // *************************************************************************
    virtual const uint8_t* GetCharGataPtr(uint8_t ch) {return &font_data_ptr[ch * GetBytesPerChar()];}

    // *************************************************************************
    // ***   GetGataPointer   **************************************************
    // *************************************************************************
    virtual const uint8_t* GetGataPointer() {return font_data_ptr;}

  protected:
    // Width and Height of character
    uint8_t char_width = 0U;
    uint8_t char_height = 0U;
    // Bytes Per Char
    uint16_t bytes_per_char = 0U;
    // Pointer to font data
    const uint8_t* font_data_ptr = nullptr;

  private:
    // Nothing yet
};

#endif
