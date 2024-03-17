//******************************************************************************
//  @file ILI9488.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: ILI9488 Low Level Driver Class, implementation
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
#include "ILI9488.h"

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************

// Commands definitions

#define CMD_NOP        0x00 // No Operation
#define CMD_SWRESET    0x01 // Software Reset
#define CMD_RDDID      0x04 // Read Display Identification Information
#define CMD_RDDST      0x09 // Read Display Status

#define CMD_RDMODE     0x0A // Read Display Power Mode
#define CMD_RDMADCTL   0x0B // Read Display MADCTL
#define CMD_RDPIXFMT   0x0C // Read Display Pixel Format
#define CMD_RDIMGFMT   0x0D // Read Display Image Format
//#define CMD_RDSIGMOD   0x0E // Read Display Signal Mode
#define CMD_RDSELFDIAG 0x0F // Read Display Self-Diagnostic Result

#define CMD_SLPIN      0x10 // Enter Sleep Mode
#define CMD_SLPOUT     0x11 // Sleep OUT
#define CMD_PTLON      0x12 // Partial Mode ON
#define CMD_NORON      0x13 // Normal Display Mode ON

#define CMD_INVOFF     0x20 // Display Inversion OFF
#define CMD_INVON      0x21 // Display Inversion ON
#define CMD_GAMMASET   0x26 // Gamma Set
#define CMD_DISPOFF    0x28 // Display OFF
#define CMD_DISPON     0x29 // Display ON

#define CMD_CASET      0x2A // Column Address Set
#define CMD_PASET      0x2B // Page Address Set
#define CMD_RAMWR      0x2C // Memory Write
//#define CMD_GSET       0x2D // Color SET
#define CMD_RAMRD      0x2E // Memory Read

#define CMD_PTLAR      0x30 // Partial Area
//#define CMD_VSCRDEF    0x33 // Vertical Scrolling Definition
//#define CMD_TELOFF     0x34 // Tearing Effect Line OFF
//#define CMD_TELON      0x35 // Tearing Effect Line ON
#define CMD_MADCTL     0x36 // Memory Access Control
//#define CMD_VSAADDR    0x37 // Vertical Scrolling Start Address
//#define CMD_IDLMOFF    0x38 // Idle Mode OFF
//#define CMD_IDLMON     0x39 // Idle Mode ON
#define CMD_PIXFMT     0x3A // Pixel Format Set

#define CMD_RGBISC     0xB0 // RGB Interface Signal Control
#define CMD_FRMCTR1    0xB1 // Frame Control (In Normal Mode)
#define CMD_FRMCTR2    0xB2 // Frame Control (In Idle Mode)
#define CMD_FRMCTR3    0xB3 // Frame Control (In Partial Mode)
#define CMD_INVCTR     0xB4 // Display Inversion Control
//#define CMD_BLKPC      0xB5 // Blanking Porch Control
#define CMD_DFUNCTR    0xB6 // Display Function Control

#define CMD_PWCTR1     0xC0 // Power Control 1
#define CMD_PWCTR2     0xC1 // Power Control 2
#define CMD_PWCTR3     0xC2
#define CMD_PWCTR4     0xC3
#define CMD_PWCTR5     0xC4
//#define CMD_PWCTR6     0xFC
#define CMD_VMCTR1     0xC5 // VCOM Control 1
#define CMD_VMCTR2     0xC7 // VCOM Control 2
//#define CMD_PWCTRA     0xCB // Power control A
//#define CMD_PWCTRB     0xCF // Power control B

//#define CMD_NVMEMWR    0xD0 // NV Memory Write
//#define CMD_NVMEMPK    0xD1 // NV Memory Protection Key
//#define CMD_NVMEMSR    0xD2 // NV Memory Status Read
//#define CMD_READID4    0xD3 // Read ID4

#define CMD_RDID1      0xDA // Read ID1
#define CMD_RDID2      0xDB // Read ID2
#define CMD_RDID3      0xDC // Read ID3
#define CMD_RDID4      0xDD

#define CMD_GMCTRP1    0xE0 // Positive Gamma Correction
#define CMD_GMCTRN1    0xE1 // Negative Gamma Correction
//#define CMD_DGCTRL1    0xE2 // Digital Gamma Control 1
//#define CMD_DGCTRL2    0xE3 // Digital Gamma Control 2
//#define CMD_DRVTMCA    0xE8 // Driver timing control A
//#define CMD_DRVTMCB    0xEA // Driver timing control B
//#define CMD_PWONSC     0xED // Power on sequence control

//#define CMD_EN3G       0xF2 // Enable 3 gamma control
//#define CMD_INTCTRL    0xF6 // Interface Control
#define CMD_PUMPRC     0xF7 // Pump ratio control

// Memory Access Control register bits definitions

#define MADCTL_MY  0x80 // Row Address Order
#define MADCTL_MX  0x40 // Column Address Order
#define MADCTL_MV  0x20 // Row / Column Exchange
#define MADCTL_ML  0x10 // Vertical Refresh Order
#define MADCTL_BGR 0x08 // BGR Order
#define MADCTL_RGB 0x00 // RGB Order (No BGR bit)
#define MADCTL_MH  0x04 // Horizontal Refresh ORDER

// Color definitions
#define ILI9488_BLACK       0x0000      //   0,   0,   0
#define ILI9488_NAVY        0x000F      //   0,   0, 128
#define ILI9488_DARKGREEN   0x03E0      //   0, 128,   0
#define ILI9488_DARKCYAN    0x03EF      //   0, 128, 128
#define ILI9488_MAROON      0x7800      // 128,   0,   0
#define ILI9488_PURPLE      0x780F      // 128,   0, 128
#define ILI9488_OLIVE       0x7BE0      // 128, 128,   0
#define ILI9488_LIGHTGREY   0xC618      // 192, 192, 192
#define ILI9488_DARKGREY    0x7BEF      // 128, 128, 128
#define ILI9488_BLUE        0x001F      //   0,   0, 255
#define ILI9488_GREEN       0x07E0      //   0, 255,   0
#define ILI9488_CYAN        0x07FF      //   0, 255, 255
#define ILI9488_RED         0xF800      // 255,   0,   0
#define ILI9488_MAGENTA     0xF81F      // 255,   0, 255
#define ILI9488_YELLOW      0xFFE0      // 255, 255,   0
#define ILI9488_WHITE       0xFFFF      // 255, 255, 255
#define ILI9488_ORANGE      0xFD20      // 255, 165,   0
#define ILI9488_GREENYELLOW 0xAFE5      // 173, 255,  47
#define ILI9488_PINK        0xF81F

// *****************************************************************************
// ***   Public: Init screen   *************************************************
// *****************************************************************************
Result ILI9488::Init(void)
{
  if(display_rst != nullptr)
  {
    // Reset sequence. Used only if GPIO pin used as LCD reset.
    display_rst->SetHigh();
    HAL_Delay(5);
    display_rst->SetLow();
    HAL_Delay(20);
    display_rst->SetHigh();
    HAL_Delay(150);
  }

  // Exit Sleep
  WriteCommand(CMD_SWRESET);
  // Delay for execute previous command
  HAL_Delay(100U);

  // Power control 1
  WriteCommand(CMD_PWCTR1);
  WriteData(0x17); // Vreg1out
  WriteData(0x15); // Verg2out

  // Power control 2
  WriteCommand(CMD_PWCTR2);
  WriteData(0x41); // VGH,VGL

  // VCM control 1
  WriteCommand(CMD_VMCTR1);
  WriteData(0x00);
  WriteData(0x12); // Vcom
  WriteData(0x80);

  // Interface Pixel Format
  WriteCommand(CMD_PIXFMT);
#if defined(COLOR_3BIT)
  WriteData(0x11); // 0x11 - 3 bit
#else
  WriteData(0x66); // 0x66 - 18 bit, 0x55 - 16 bit(DOESN'T WORK!), 0x11 - 3 bit
#endif

  // Frame Control (In Normal Mode)
  WriteCommand(CMD_FRMCTR1);
  WriteData(0xA0); // Frame rate 60Hz

  // Pump ratio control
  WriteCommand(CMD_PUMPRC);
  WriteData(0xA9);
  WriteData(0x51);
  WriteData(0x2C);
  WriteData(0x82);    // D7 stream, loose

  // Memory Access Control
  WriteCommand(CMD_MADCTL);
  WriteData(MADCTL_MV | MADCTL_BGR);

  // Positive Gamma Correction
  WriteCommand(CMD_GMCTRP1);
  WriteData(0x00);
  WriteData(0x03);
  WriteData(0x09);
  WriteData(0x08);
  WriteData(0x16);
  WriteData(0x0A);
  WriteData(0x3F);
  WriteData(0x78);
  WriteData(0x4C);
  WriteData(0x09);
  WriteData(0x0A);
  WriteData(0x08);
  WriteData(0x16);
  WriteData(0x1A);
  WriteData(0x0F);

  // Negative Gamma Correction
  WriteCommand(CMD_GMCTRN1);
  WriteData(0x00);
  WriteData(0x16);
  WriteData(0x19);
  WriteData(0x03);
  WriteData(0x0F);
  WriteData(0x05);
  WriteData(0x32);
  WriteData(0x45);
  WriteData(0x46);
  WriteData(0x04);
  WriteData(0x0E);
  WriteData(0x0D);
  WriteData(0x35);
  WriteData(0x37);
  WriteData(0x0F);

  // Interface Mode Control
  WriteCommand(CMD_RGBISC);
  WriteData(0x80); // SDO NOT USE

  // Display Inversion Control
  WriteCommand(CMD_INVCTR);
  WriteData(0x02); // 2-dot

  // Display Function Control RGB/MCU Interface Control
  WriteCommand(CMD_DFUNCTR);
  WriteData(0x02); // MCU
  WriteData(0x02); // Source,Gate scan dieection

  WriteCommand(0xE9); // Set Image Function
  WriteData(0x00);    // Disable 24 bit data

  // Exit Sleep
  WriteCommand(CMD_SLPOUT);
  // Delay for execute previous command
  HAL_Delay(120U);
  // Display on
  WriteCommand(CMD_DISPON);

  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Public: Prepare data (32 bit -> 24 bit)   *****************************
// *****************************************************************************
Result ILI9488::PrepareData(uint32_t* data, uint32_t n)
{
  // Change pointer to uint8_t
  uint8_t* dt = (uint8_t*)data;
  // Index for packed data
  uint32_t idx = 0;
  // Do packing 32 bit -> 24 bit
  for(uint32_t i = 0u; i < n*4; i++)
  {
    // Create 24-bit RGB values from 32 bit
    dt[idx++] = dt[i++];
    dt[idx++] = dt[i++];
    dt[idx++] = dt[i++];
  }
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Public: Prepare data (16 bit -> 18 bit)   *****************************
// *****************************************************************************
Result ILI9488::PrepareData(uint16_t* data, uint32_t n)
{
  // Change pointer to uint8_t
  uint8_t* dt = (uint8_t*)data;
  // Index variable
  uint16_t idx = (n * 3u) - 1u;
  // Convert data
  for(int16_t i = n - 1u; i >= 0; i--)
  {
    uint16_t color = dt[i*2u+1u] | (dt[i*2u] << 8u);
    // Create 24-bit RGB values from 16 bit
    dt[idx--] = (color & 0x001F) << 3;
    dt[idx--] = (color & 0x07E0) >> 3; // >> 5 << 3
    dt[idx--] = (color & 0xF800) >> 8; // >> 11 << 3
  }
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Public: Prepare data (16 bit -> 3 bit)   ******************************
// *****************************************************************************
Result ILI9488::PrepareData(uint8_t* data, uint32_t n)
{
  // Convert data
  for(uint32_t i = 0u; i < n / 2u; i++)
  {
//    uint16_t color1 = data[i*4u+1u] | (data[i*4u+0u] << 8u);
//    uint16_t color2 = data[i*4u+3u] | (data[i*4u+2u] << 8u);
//    // Create two 3-bit RGB values from two 16 bit
//    data[i] = ((color1 & 0x0010) ? 0x20 : 0x00) | ((color1 & 0x0400) ? 0x10 : 0x00) | ((color1 & 0x8000) ? 0x08 : 0x00) |
//              ((color2 & 0x0010) ? 0x04 : 0x00) | ((color2 & 0x0400) ? 0x02 : 0x00) | ((color2 & 0x8000) ? 0x01 : 0x00);

//    data[i] = ((data[i*4u+1u] & 0x10) ? 0x20 : 0x00) | ((data[i*4u+0u] & 0x04) ? 0x10 : 0x00) | ((data[i*4u+0u] & 0x80) ? 0x08 : 0x00) |
//              ((data[i*4u+3u] & 0x10) ? 0x04 : 0x00) | ((data[i*4u+2u] & 0x04) ? 0x02 : 0x00) | ((data[i*4u+2u] & 0x80) ? 0x01 : 0x00);

//    data[i] = ((data[i*4u+1u] & 0x10) << 1u) | ((data[i*4u+0u] & 0x04) << 2u) | ((data[i*4u+0u] & 0x80) >> 4u) |
//              ((data[i*4u+3u] & 0x10) >> 2u) | ((data[i*4u+2u] & 0x04) >> 1u) | ((data[i*4u+2u] & 0x80) >> 7u);

    //
    data[i] = ((data[i*2u + 0u] & 0x07) << 3u) | (data[i*2u + 1u] & 0x07);
  }
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Public: Write data steram to SPI   ************************************
// *****************************************************************************
Result ILI9488::WriteDataStream(uint8_t* data, uint32_t n)
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
bool ILI9488::IsTransferComplete(void)
{
  return spi.IsTransferComplete();
}

// *****************************************************************************
// ***   Pull up CS line for LCD  **********************************************
// *****************************************************************************
Result ILI9488::StopTransfer(void)
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
Result ILI9488::SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  WriteCommand(CMD_CASET); // Column address set
  WriteData(x0 >> 8);
  WriteData(x0 & 0xFF); // XSTART 
  WriteData(x1 >> 8);
  WriteData(x1 & 0xFF); // XEND

  WriteCommand(CMD_PASET); // Row address set
  WriteData(y0 >> 8);
  WriteData(y0 & 0xFF); // YSTART
  WriteData(y1 >> 8);
  WriteData(y1 & 0xFF); // YEND

  WriteCommand(CMD_RAMWR); // write to RAM

  // Prepare for write data
  display_dc.SetHigh(); // Data
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Set screen orientation   **********************************************
// *****************************************************************************
Result ILI9488::SetRotation(IDisplay::Rotation r)
{
  rotation = r;
  WriteCommand(CMD_MADCTL);
  switch (rotation)
  {
    case IDisplay::ROTATION_BOTTOM:
      WriteData(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
      width  = init_width;
      height = init_height;
      break;

    case IDisplay::ROTATION_RIGHT:
      WriteData(MADCTL_MY | MADCTL_BGR);
      width  = init_height;
      height = init_width;
      break;

    case IDisplay::ROTATION_LEFT:
      WriteData(MADCTL_MX | MADCTL_BGR);
      width  = init_height;
      height = init_width;
      break;

    case IDisplay::ROTATION_TOP:
      WriteData(MADCTL_MV | MADCTL_BGR);
      width  = init_width;
      height = init_height;
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
Result ILI9488::PushColor(uint32_t color)
{
  display_dc.SetHigh(); // Data

  // Write color
  SpiWrite(((uint8_t*)&color)[0u]);
  SpiWrite(((uint8_t*)&color)[1u]);
  SpiWrite(((uint8_t*)&color)[2u]);

  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Write color to screen   ***********************************************
// *****************************************************************************
Result ILI9488::PushColor(uint16_t color)
{
  display_dc.SetHigh(); // Data

  // Create 24-bit RGB values from 16 bit
  uint8_t r = (color & 0xF800) >> 11;
  uint8_t g = (color & 0x07E0) >> 5;
  uint8_t b = color & 0x001F;
  r = (r * 0xFF) / 31;
  g = (g * 0xFF) / 63;
  b = (b * 0xFF) / 31;
  // Write color
  SpiWrite(r);
  SpiWrite(g);
  SpiWrite(b);

  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Write color to screen   ***********************************************
// *****************************************************************************
Result ILI9488::PushColor(uint8_t color)
{
  display_dc.SetHigh(); // Data

  // Write color
  SpiWrite(color);

  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Draw one pixel on  screen   *******************************************
// *****************************************************************************
Result ILI9488::DrawPixel(int16_t x, int16_t y, color_t color)
{
  if((x >= 0) && (x < width) && (y >= 0) && (y < height))
  {
    SetAddrWindow(x,y,x+1,y+1);
    // Write color
    PushColor(color);
  }
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Draw vertical line   **************************************************
// *****************************************************************************
Result ILI9488::DrawFastVLine(int16_t x, int16_t y, int16_t h, color_t color)
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
Result ILI9488::DrawFastHLine(int16_t x, int16_t y, int16_t w, color_t color)
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
Result ILI9488::FillRect(int16_t x, int16_t y, int16_t w, int16_t h, color_t color)
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
Result ILI9488::InvertDisplay(bool invert)
{
  WriteCommand(invert ? CMD_INVON : CMD_INVOFF);
  // Always Ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Read data from SPI   **************************************************
// *****************************************************************************
inline uint8_t ILI9488::SpiRead(void)
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
inline uint8_t ILI9488::ReadData(void)
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
uint8_t ILI9488::ReadCommand(uint8_t c)
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
inline void ILI9488::SpiWrite(uint8_t c)
{
  display_cs.SetLow(); // Pull down CS
  spi.Write(&c, sizeof(c));
  display_cs.SetHigh(); // Pull up CS
}

// *****************************************************************************
// ***   Write command to SPI   ************************************************
// *****************************************************************************
inline void ILI9488::WriteCommand(uint8_t c)
{
  display_dc.SetLow(); // Command
  SpiWrite(c);
}

// *****************************************************************************
// ***   Write data to SPI   ***************************************************
// *****************************************************************************
inline void ILI9488::WriteData(uint8_t c)
{
  display_dc.SetHigh(); // Data
  SpiWrite(c);
}
