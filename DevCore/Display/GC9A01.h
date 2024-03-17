//******************************************************************************
//  @file GC9A01.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: GC9A01 Low Level Driver Class, header
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

#ifndef GC9A01_h
#define GC9A01_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include <DevCfg.h>
#include "IDisplay.h"
#include "ISpi.h"
#include "IGpio.h"

// *****************************************************************************
// ***   GC9A01   *************************************************************
// *****************************************************************************
class GC9A01 : public IDisplay
{
  public:
    // *************************************************************************
    // ***   Constructor   *****************************************************
    // *************************************************************************
    explicit GC9A01(int32_t in_width, int32_t in_height, ISpi& in_spi, IGpio& disp_cs, IGpio& disp_dc, IGpio* disp_rst = nullptr) :
      IDisplay(in_width, in_height, 2), spi(in_spi), display_cs(disp_cs), display_dc(disp_dc), display_rst(disp_rst) {};

    // *************************************************************************
    // ***   Init screen   *****************************************************
    // *************************************************************************
    virtual Result Init(void);

    // *************************************************************************
    // ***   Write data steram to SPI   ****************************************
    // *************************************************************************
    virtual Result WriteDataStream(uint8_t* data, uint32_t n);

    // *************************************************************************
    // ***   Check SPI transfer status  ****************************************
    // *************************************************************************
    virtual bool IsTransferComplete(void);

    // *************************************************************************
    // ***   Pull up CS line for LCD  ******************************************
    // *************************************************************************
    virtual Result StopTransfer(void);

    // *************************************************************************
    // ***   Set output window   ***********************************************
    // *************************************************************************
    virtual Result SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

    // *************************************************************************
    // ***   Set screen orientation   ******************************************
    // *************************************************************************
    virtual Result SetRotation(IDisplay::Rotation r);

    // *************************************************************************
    // ***   Write color to screen   *******************************************
    // *************************************************************************
    virtual Result PushColor(color_t color);

    // *************************************************************************
    // ***   Draw one pixel on  screen   ***************************************
    // *************************************************************************
    virtual Result DrawPixel(int16_t x, int16_t y, color_t color);

    // *************************************************************************
    // ***   Draw vertical line   **********************************************
    // *************************************************************************
    virtual Result DrawFastVLine(int16_t x, int16_t y, int16_t h, color_t color);

    // *************************************************************************
    // ***   Draw horizontal line   ********************************************
    // *************************************************************************
    virtual Result DrawFastHLine(int16_t x, int16_t y, int16_t w, color_t color);

    // *************************************************************************
    // ***   Fill rectangle on screen   ****************************************
    // *************************************************************************
    virtual Result FillRect(int16_t x, int16_t y, int16_t w, int16_t h, color_t color);

    // *************************************************************************
    // ***   Invert display   **************************************************
    // *************************************************************************
    virtual Result InvertDisplay(bool invert);

  private:
    // Handle to SPI used for display
    ISpi& spi;
    // Reference to CS and DC - mandatory
    IGpio& display_cs;
    IGpio& display_dc;
    // Pointer to Reset - optional
    IGpio* display_rst = nullptr;

    // *************************************************************************
    // ***   Private: Write data to SPI   **************************************
    // *************************************************************************
    inline void WriteData(uint8_t c);

    // *************************************************************************
    // ***   Private: Write command to SPI   ***********************************
    // *************************************************************************
    inline void WriteCommand(uint8_t c);

    // *************************************************************************
    // ***   Private: Write byte to SPI   **************************************
    // *************************************************************************
    inline void SpiWrite(uint8_t c);

    // *************************************************************************
    // ***   Private: Read data from SPI   *************************************
    // *************************************************************************
    inline uint8_t SpiRead(void);

    // *************************************************************************
    // ***   Private: Read data from display   *********************************
    // *************************************************************************
    inline uint8_t ReadData(void);

    // *************************************************************************
    // ***   Private: Send read command ad read result   ***********************
    // *************************************************************************
    uint8_t ReadCommand(uint8_t c);
};

#endif
