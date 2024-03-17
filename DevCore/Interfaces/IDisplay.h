//******************************************************************************
//  @file IDisplay.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: Display driver interface, header
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

#ifndef IDisplay_h
#define IDisplay_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include <DevCfg.h>

// *****************************************************************************
// ***   IDisplay   ************************************************************
// *****************************************************************************
class IDisplay
{
  public:
    // *************************************************************************
    // ***   Display rotation: should be the same as ITouchscreen !   **********
    // *************************************************************************
    enum Rotation
    {
      ROTATION_TOP = 0,
      ROTATION_LEFT,
      ROTATION_BOTTOM,
      ROTATION_RIGHT,
      ROTATION_CNT
    };

    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    explicit IDisplay(int32_t in_width, int32_t in_height, int32_t in_byte_per_pixel) :
    byte_per_pixel(in_byte_per_pixel), init_width(in_width), init_height(in_height), width(in_width), height(in_height) {};

    // *************************************************************************
    // ***   Public: Destructor   **********************************************
    // *************************************************************************
    virtual ~IDisplay() {};

    // *************************************************************************
    // ***   Public: Init screen   *********************************************
    // *************************************************************************
    virtual Result Init(void) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Prepare data   ********************************************
    // *************************************************************************
    virtual Result PrepareData(color_t* data, uint32_t n)  {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Write data stream to Display   ****************************
    // *************************************************************************
    virtual Result WriteDataStream(uint8_t* data, uint32_t n) = 0;

    // *************************************************************************
    // ***   Public: Check transfer status  ************************************
    // *************************************************************************
    virtual bool IsTransferComplete(void) = 0;

    // *************************************************************************
    // ***   Public: Stop transfer(i.e. Pull up CS line for LCD)  **************
    // *************************************************************************
    virtual Result StopTransfer(void) = 0;

    // *************************************************************************
    // ***   Public: Set output window   ***************************************
    // *************************************************************************
    virtual Result SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Set screen orientation   **********************************
    // *************************************************************************
    virtual Result SetRotation(Rotation r) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Write color to screen   ***********************************
    // *************************************************************************
    virtual Result PushColor(color_t color) = 0;

    // *************************************************************************
    // ***   Public: Draw one pixel on  screen   *******************************
    // *************************************************************************
    virtual Result DrawPixel(int16_t x, int16_t y, color_t color) = 0;

    // *************************************************************************
    // ***   Public: Draw vertical line   **************************************
    // *************************************************************************
    virtual Result DrawFastVLine(int16_t x, int16_t y, int16_t h, color_t color) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Draw horizontal line   ************************************
    // *************************************************************************
    virtual Result DrawFastHLine(int16_t x, int16_t y, int16_t w, color_t color) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Fill full screen   ****************************************
    // *************************************************************************
    virtual Result FillScreen(color_t color) {return FillRect(0, 0, width, height, color);}

    // *************************************************************************
    // ***   Public: Fill rectangle on screen   ********************************
    // *************************************************************************
    virtual Result FillRect(int16_t x, int16_t y, int16_t w, int16_t h, color_t color) = 0;

    // *************************************************************************
    // ***   Public: Invert display   ******************************************
    // *************************************************************************
    virtual Result InvertDisplay(bool invert) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Return if data have to be prepared before send to display   *******
    // *************************************************************************
    virtual bool IsDataNeedPreparation(void) {return false;}

    // *************************************************************************
    // ***   Return byte(s) count for given number of pixels   *****************
    // *************************************************************************
    virtual int32_t GetPixelDataCnt(uint16_t pixel_cnt) {return(pixel_cnt * byte_per_pixel);}

    // *************************************************************************
    // ***   Public: Return screen width   *************************************
    // *************************************************************************
    inline int32_t GetWidth(void) {return width;}

    // *************************************************************************
    // ***   Public: Return screen height   ************************************
    // *************************************************************************
    inline int32_t GetHeight(void) {return height;}

    // *************************************************************************
    // ***   Return byte(s) per pixel   ****************************************
    // *************************************************************************
    inline int32_t GetBytesPerPixel(void) {return byte_per_pixel;}

    // *************************************************************************
    // ***   Public: Pass 8-bit (each) R,G,B, get back 16-bit packed color   ***
    // *************************************************************************
    uint16_t GetColor565(uint8_t r, uint8_t g, uint8_t b)
    {
      return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

  protected:

    // Byte(s) per pixel
    const int32_t byte_per_pixel = 0u;
    // Width during initialization(need because of rotation)
    const int32_t init_width = 0U;
    // Height during initialization(need because of rotation)
    const int32_t init_height = 0U;
    // Current width
    int32_t width = 0U;
    // Current height
    int32_t height = 0U;
    // Rotation
    Rotation rotation = ROTATION_TOP;

  private:
    // *************************************************************************
    // ***   Private: Constructors and assign operator - prevent copying   *****
    // *************************************************************************
    IDisplay(const IDisplay&);
};

#endif
