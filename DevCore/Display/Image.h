//******************************************************************************
//  @file Image.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: Image Visual Object Class, header
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

#ifndef Images_h
#define Images_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "VisObject.h"

// *****************************************************************************
// ***   Palettes external   ***************************************************
// *****************************************************************************
extern const color_t PALETTE_884[256];
extern const color_t PALETTE_775[256];
extern const color_t PALETTE_676[256];

// *****************************************************************************
// ***   Image description structure   *****************************************
// *****************************************************************************
typedef struct typeImageDesc
{
  // Image width
  uint16_t width;
  // Image height
  uint16_t height;
  // Bits per pixel
  uint8_t bits_per_pixel;
  // Pointer to image data
  union
  {
    const uint8_t*  imgp;
    const color_t*  imgb;
    const void*     img;
  };
  // Pointer to palette
  const color_t* palette;
  // Transparent color (-1 no transparent colors)
  int32_t transparent_color;
} ImageDesc;

// *****************************************************************************
// ***   Universal Image Class   ***********************************************
// *****************************************************************************
class Image : public VisObject
{
  public:
    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    Image() {};

    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    Image(int32_t x, int32_t y, const ImageDesc& img_dsc);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t y = 0);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t x = 0);

    // *************************************************************************
    // ***   Set Horizontal Flip function   ************************************
    // *************************************************************************
    void SetHorizontalFlip(bool flip) {hor_mirror = flip;}

    // *************************************************************************
    // ***   Set Image function   **********************************************
    // *************************************************************************
    void SetImage(const ImageDesc& img_dsc, bool semaphore_taken = false);

  protected:
    // Bits per pixel
    uint8_t bits_per_pixel = 0u;
    // Pointer to the image
    const void* img = nullptr;
    // Pointer to the palette
    const color_t* palette = nullptr;
    // Transparent color (-1 no transparent colors)
    int32_t transparent_color = -1;
    // Horizontal mirror
    bool hor_mirror = false;
};

// *****************************************************************************
// ***   Palette Image Class   *************************************************
// *****************************************************************************
class ImagePalette : public VisObject
{
  public:
    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    ImagePalette(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t* p_img, const color_t* p_palette);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t y = 0);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t x = 0);

  private:
    // Pointer to the image
    const uint8_t* img;
    // Pointer to the palette
    const color_t* palette;
};

// *****************************************************************************
// ***   Bitmap Image Class   **************************************************
// *****************************************************************************
class ImageBitmap : public VisObject
{
  public:
    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    ImageBitmap(int32_t x, int32_t y, int32_t w, int32_t h, const color_t* p_img);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void DrawInBufH(color_t* buf, int32_t n, int32_t row, int32_t y = 0);

    // *************************************************************************
    // ***   Put line in buffer   **********************************************
    // *************************************************************************
    virtual void DrawInBufW(color_t* buf, int32_t n, int32_t line, int32_t x = 0);

  private:
    // Pointer to the image
    const color_t* img;
};

#endif
