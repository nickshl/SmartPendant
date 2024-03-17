//******************************************************************************
//  @file GC9A01.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: GC9A01 Low Level Driver Class, implementation
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
#include "GC9A01.h"

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************

// *****************************************************************************
// ***   Public: Init screen   *************************************************
// *****************************************************************************
Result GC9A01::Init(void)
{
  if(display_rst != nullptr)
  {
    // Reset sequence. Used only if GPIO pin used as LCD reset.
    display_rst->SetHigh();
    HAL_Delay(100);
    display_rst->SetLow();
    HAL_Delay(100);
    display_rst->SetHigh();
    HAL_Delay(100);
  }

  // Set the resolution and scanning method of the screen
  // Set the read / write scan direction of the frame memory
  WriteCommand(0x36); // MX, MY, RGB mode
  WriteData(0XC8);    // 0x08 set RGB
  
  // Set the initialization register
  WriteCommand(0xEF);
  WriteCommand(0xEB);
  WriteData(0x14); 

  WriteCommand(0xFE);
  WriteCommand(0xEF);

  WriteCommand(0xEB);
  WriteData(0x14); 

  WriteCommand(0x84);
  WriteData(0x40); 

  WriteCommand(0x85);
  WriteData(0xFF); 

  WriteCommand(0x86);
  WriteData(0xFF); 

  WriteCommand(0x87);
  WriteData(0xFF);

  WriteCommand(0x88);
  WriteData(0x0A);

  WriteCommand(0x89);
  WriteData(0x21); 

  WriteCommand(0x8A);
  WriteData(0x00); 

  WriteCommand(0x8B);
  WriteData(0x80); 

  WriteCommand(0x8C);
  WriteData(0x01); 

  WriteCommand(0x8D);
  WriteData(0x01); 

  WriteCommand(0x8E);
  WriteData(0xFF); 

  WriteCommand(0x8F);
  WriteData(0xFF); 


  WriteCommand(0xB6);
  WriteData(0x00);
  WriteData(0x20);

  WriteCommand(0x36);
  WriteData(0x08); // Set as vertical screen

  WriteCommand(0x3A);
  WriteData(0x05); 


  WriteCommand(0x90);
  WriteData(0x08);
  WriteData(0x08);
  WriteData(0x08);
  WriteData(0x08);

  WriteCommand(0xBD);
  WriteData(0x06);

  WriteCommand(0xBC);
  WriteData(0x00);

  WriteCommand(0xFF);
  WriteData(0x60);
  WriteData(0x01);
  WriteData(0x04);

  WriteCommand(0xC3);
  WriteData(0x13);
  WriteCommand(0xC4);
  WriteData(0x13);

  WriteCommand(0xC9);
  WriteData(0x22);

  WriteCommand(0xBE);
  WriteData(0x11);

  WriteCommand(0xE1);
  WriteData(0x10);
  WriteData(0x0E);

  WriteCommand(0xDF);
  WriteData(0x21);
  WriteData(0x0c);
  WriteData(0x02);

  WriteCommand(0xF0);   
  WriteData(0x45);
  WriteData(0x09);
  WriteData(0x08);
  WriteData(0x08);
  WriteData(0x26);
  WriteData(0x2A);

  WriteCommand(0xF1);    
  WriteData(0x43);
  WriteData(0x70);
  WriteData(0x72);
  WriteData(0x36);
  WriteData(0x37);  
  WriteData(0x6F);


  WriteCommand(0xF2);   
  WriteData(0x45);
  WriteData(0x09);
  WriteData(0x08);
  WriteData(0x08);
  WriteData(0x26);
  WriteData(0x2A);

  WriteCommand(0xF3);   
  WriteData(0x43);
  WriteData(0x70);
  WriteData(0x72);
  WriteData(0x36);
  WriteData(0x37); 
  WriteData(0x6F);

  WriteCommand(0xED);    
  WriteData(0x1B); 
  WriteData(0x0B); 

  WriteCommand(0xAE);
  WriteData(0x77);

  WriteCommand(0xCD);
  WriteData(0x63);


  WriteCommand(0x70);
  WriteData(0x07);
  WriteData(0x07);
  WriteData(0x04);
  WriteData(0x0E); 
  WriteData(0x0F); 
  WriteData(0x09);
  WriteData(0x07);
  WriteData(0x08);
  WriteData(0x03);

  WriteCommand(0xE8);
  WriteData(0x34);

  WriteCommand(0x62);
  WriteData(0x18);
  WriteData(0x0D);
  WriteData(0x71);
  WriteData(0xED);
  WriteData(0x70); 
  WriteData(0x70);
  WriteData(0x18);
  WriteData(0x0F);
  WriteData(0x71);
  WriteData(0xEF);
  WriteData(0x70); 
  WriteData(0x70);

  WriteCommand(0x63);
  WriteData(0x18);
  WriteData(0x11);
  WriteData(0x71);
  WriteData(0xF1);
  WriteData(0x70); 
  WriteData(0x70);
  WriteData(0x18);
  WriteData(0x13);
  WriteData(0x71);
  WriteData(0xF3);
  WriteData(0x70); 
  WriteData(0x70);

  WriteCommand(0x64);
  WriteData(0x28);
  WriteData(0x29);
  WriteData(0xF1);
  WriteData(0x01);
  WriteData(0xF1);
  WriteData(0x00);
  WriteData(0x07);

  WriteCommand(0x66);
  WriteData(0x3C);
  WriteData(0x00);
  WriteData(0xCD);
  WriteData(0x67);
  WriteData(0x45);
  WriteData(0x45);
  WriteData(0x10);
  WriteData(0x00);
  WriteData(0x00);
  WriteData(0x00);

  WriteCommand(0x67);
  WriteData(0x00);
  WriteData(0x3C);
  WriteData(0x00);
  WriteData(0x00);
  WriteData(0x00);
  WriteData(0x01);
  WriteData(0x54);
  WriteData(0x10);
  WriteData(0x32);
  WriteData(0x98);

  WriteCommand(0x74);
  WriteData(0x10);    
  WriteData(0x85);    
  WriteData(0x80);
  WriteData(0x00); 
  WriteData(0x00); 
  WriteData(0x4E);
  WriteData(0x00);        

  WriteCommand(0x98);
  WriteData(0x3e);
  WriteData(0x07);

  WriteCommand(0x35);    
  WriteCommand(0x21);

  WriteCommand(0x11);
  HAL_Delay(120);
  WriteCommand(0x29);
  HAL_Delay(20U);

  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Public: Write data steram to SPI   ************************************
// *****************************************************************************
Result GC9A01::WriteDataStream(uint8_t* data, uint32_t n)
{
  // Data
  display_dc.SetHigh();
  // Pull down CS
  display_cs.SetLow();
  // Send data to screen
  Result result = spi.WriteAsync(data, n);
  // Return result
  return result;
}

// *****************************************************************************
// ***   Public:  Check SPI transfer status   **********************************
// *****************************************************************************
bool GC9A01::IsTransferComplete(void)
{
  return spi.IsTransferComplete();
}

// *****************************************************************************
// ***   Pull up CS line for LCD  **********************************************
// *****************************************************************************
Result GC9A01::StopTransfer(void)
{
  // In case if transfer isn't finished - abort it.
  if(spi.IsTransferComplete() == false)
  {
    spi.Abort();
  }
  // Pull up CS
  display_cs.SetHigh();
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Set output window   ***************************************************
// *****************************************************************************
Result GC9A01::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  // Set the X coordinates
  WriteCommand(0x2A);
  WriteData(x0 >> 8);
  WriteData(x0 & 0xFF);
  WriteData(x1 >> 8);
  WriteData(x1 & 0xFF);

  // Set the Y coordinates
  WriteCommand(0x2B);
  WriteData(y0 >> 8);
  WriteData(y0 & 0xFF);
  WriteData(y1 >> 8);
  WriteData(y1 & 0xFF);

  WriteCommand(0x2C);

  // Prepare for write data
  display_dc.SetHigh(); // Data

  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Set screen orientation   **********************************************
// *****************************************************************************
Result GC9A01::SetRotation(IDisplay::Rotation r)
{
  rotation = r;
//  WriteCommand(CMD_MADCTL);
  switch (rotation)
  {
    case IDisplay::ROTATION_BOTTOM:
//      WriteData(MADCTL_BGR);
//      width  = init_height;
//      height = init_width;
      break;

    case IDisplay::ROTATION_RIGHT:
//      WriteData(MADCTL_MV | MADCTL_BGR);
//      width  = init_width;
//      height = init_height;
      break;

    case IDisplay::ROTATION_LEFT: // Y: up -> down
//      WriteData(MADCTL_MX | MADCTL_MY | MADCTL_BGR);
//      width  = init_height;
//      height = init_width;
      break;

    case IDisplay::ROTATION_TOP: // X: left -> right
//      WriteData(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
//      width  = init_width;
//      height = init_height;
      break;

    default:
      break;
  }
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Write color to screen   ***********************************************
// *****************************************************************************
Result GC9A01::PushColor(color_t color)
{
  display_dc.SetHigh(); // Data
  // Write color
  SpiWrite(color >> 8);
  SpiWrite(color);
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Draw one pixel on  screen   *******************************************
// *****************************************************************************
Result GC9A01::DrawPixel(int16_t x, int16_t y, color_t color)
{
  if((x >= 0) && (x < width) && (y >= 0) && (y < height))
  {
    SetAddrWindow(x,y,x+1,y+1);
    // Write color
    SpiWrite(color >> 8);
    SpiWrite(color);
  }
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Draw vertical line   **************************************************
// *****************************************************************************
Result GC9A01::DrawFastVLine(int16_t x, int16_t y, int16_t h, color_t color)
{
  // Rudimentary clipping
  if((x < width) && (y < height))
  {
    if((y+h-1) >= height) h = height-y;

    SetAddrWindow(x, y, x, y+h-1);

    // Swap bytes
    uint8_t clr = ((color >> 8) & 0x00FF) | ((color << 8) & 0xFF00);

    display_cs.SetLow(); // Pull down CS
    while(h--)
    {
      spi.Write(&clr, sizeof(clr));
    }
    display_cs.SetHigh(); // Pull up CS
  }
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Draw horizontal line   ************************************************
// *****************************************************************************
Result GC9A01::DrawFastHLine(int16_t x, int16_t y, int16_t w, color_t color)
{
  if((x < width) && (y < height))
  {
    if((x+w-1) >= width)  w = width-x;

    SetAddrWindow(x, y, x+w-1, y);

    // Swap bytes
    uint8_t clr = ((color >> 8) & 0x00FF) | ((color << 8) & 0xFF00);

    display_cs.SetLow(); // Pull down CS
    while(w--)
    {
      spi.Write(&clr, sizeof(clr));
    }
    display_cs.SetHigh(); // Pull up CS
  }
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Fill rectangle on screen   ********************************************
// *****************************************************************************
Result GC9A01::FillRect(int16_t x, int16_t y, int16_t w, int16_t h, color_t color)
{
  if((x < width) && (y < height))
  {
    if((x + w - 1) >= width)  w = width  - x;
    if((y + h - 1) >= height) h = height - y;

    SetAddrWindow(x, y, x+w-1, y+h-1);

    // Swap bytes
    uint8_t clr = ((color >> 8) & 0x00FF) | ((color << 8) & 0xFF00);

    display_cs.SetLow(); // Pull down CS
    for(y=h; y>0; y--)
    {
      for(x=w; x>0; x--)
      {
        spi.Write(&clr, sizeof(clr));
      }
    }
    display_cs.SetHigh(); // Pull up CS
  }
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Invert display   ******************************************************
// *****************************************************************************
Result GC9A01::InvertDisplay(bool invert)
{
//  WriteCommand(invert ? CMD_INVON : CMD_INVOFF);
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Read data from SPI   **************************************************
// *****************************************************************************
inline uint8_t GC9A01::SpiRead(void)
{
  // Result variable
  uint8_t r = 0;
  // Pull down CS
  display_cs.SetLow();
  // Receive data
  spi.Read(&r, sizeof(r));
  // Pull up CS
  display_cs.SetHigh();
  // Return result
  return r;
}

// *****************************************************************************
// ***   Read data from display   **********************************************
// *****************************************************************************
inline uint8_t GC9A01::ReadData(void)
{
  // Data
  display_dc.SetHigh(); // Data
  // Receive data
  uint8_t r = SpiRead();
  // Return result
  return r;
}

// *****************************************************************************
// ***   Send read command ad read result   ************************************
// *****************************************************************************
uint8_t GC9A01::ReadCommand(uint8_t c)
{
  // Set command mode
  display_dc.SetLow(); // Command
  SpiWrite(c);

  // Set data mode
  display_dc.SetHigh(); // Data
  // Receive data
  uint8_t r = SpiRead();

  // Return result
  return r;
}

// *****************************************************************************
// ***   Write byte to SPI   ***************************************************
// *****************************************************************************
inline void GC9A01::SpiWrite(uint8_t c)
{
  display_cs.SetLow(); // Pull down CS
  spi.Write(&c, sizeof(c));
  display_cs.SetHigh(); // Pull up CS
}

// *****************************************************************************
// ***   Write command to SPI   ************************************************
// *****************************************************************************
inline void GC9A01::WriteCommand(uint8_t c)
{
  display_dc.SetLow(); // Command
  SpiWrite(c);
}

// *****************************************************************************
// ***   Write data to SPI   ***************************************************
// *****************************************************************************
inline void GC9A01::WriteData(uint8_t c)
{
  display_dc.SetHigh(); // Data
  SpiWrite(c);
}
