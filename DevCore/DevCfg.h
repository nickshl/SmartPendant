//******************************************************************************
//  @file DevCfg.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: Config file, header
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

#ifndef DevCfg_h
#define DevCfg_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "Result.h"
#include "Rtos.h"

// Include for all hardware stuff
#include "main.h"
#include "stm32f4xx.h"

// ***   ADC   *****************************************************************
#ifdef HAL_ADC_MODULE_ENABLED
#include "adc.h"
#else
typedef uint32_t ADC_HandleTypeDef; // Dummy ADC handle for compilation
#endif
// ***   USART   ***************************************************************
#if defined(HAL_USART_MODULE_ENABLED) || defined(HAL_UART_MODULE_ENABLED)
#include "usart.h"
#else
typedef uint32_t UART_HandleTypeDef; // Dummy UART handle for compilation
#endif
// ***   SPI   *****************************************************************
#ifdef HAL_SPI_MODULE_ENABLED
#include "spi.h"
#else
typedef uint32_t SPI_HandleTypeDef; // Dummy SPI handle for compilation
#endif
// ***   I2C   *****************************************************************
#ifdef HAL_I2C_MODULE_ENABLED
#include "i2c.h"
#else
typedef uint32_t I2C_HandleTypeDef; // Dummy I2C handle for compilation
#endif
// ***   TIM   *****************************************************************
#ifdef HAL_TIM_MODULE_ENABLED
#include "tim.h"
#else
typedef uint32_t TIM_HandleTypeDef; // Dummy TIM handle for compilation
#endif
// ***   DAC   *****************************************************************
#ifdef HAL_DAC_MODULE_ENABLED
#include "dac.h"
#else
typedef uint32_t DAC_HandleTypeDef; // Dummy DAC handle for compilation
#endif

// *****************************************************************************
// ***   Configuration   *******************************************************
// *****************************************************************************

//#define DWT_ENABLED
#define UITASK_ENABLED
//#define INPUTDRV_ENABLED
#define SOUNDDRV_ENABLED

// ***   TIM Handles   *********************************************************
#if defined(HAL_TIM_MODULE_ENABLED)
  #if defined(SOUNDDRV_ENABLED)
    // Sound Timer handle
    static TIM_HandleTypeDef* const SOUND_HTIM = &htim1;
    // Sound Timer channel
    static const uint32_t SOUND_CHANNEL = TIM_CHANNEL_1;
  #endif
#endif

// *** Applications tasks stack sizes   ****************************************
const static uint16_t APPLICATION_TASK_STACK_SIZE = 1024u;
const static uint16_t EXAMPLE_MSG_TASK_STACK_SIZE = configMINIMAL_STACK_SIZE;
// *** Applications tasks priorities   *****************************************
const static uint8_t APPLICATION_TASK_PRIORITY = tskIDLE_PRIORITY + 2U;
const static uint8_t EXAMPLE_MSG_TASK_PRIORITY = tskIDLE_PRIORITY + 2U;
// *****************************************************************************

// *** System tasks stack sizes   **********************************************
const static uint16_t DISPLAY_DRV_TASK_STACK_SIZE = 1024u;
const static uint16_t UI_TASK_STACK_SIZE          = configMINIMAL_STACK_SIZE;
const static uint16_t INPUT_DRV_TASK_STACK_SIZE   = configMINIMAL_STACK_SIZE;
const static uint16_t SOUND_DRV_TASK_STACK_SIZE   = configMINIMAL_STACK_SIZE;
// *** System tasks priorities   ***********************************************
const static uint8_t DISPLAY_DRV_TASK_PRIORITY = tskIDLE_PRIORITY + 1U;
const static uint8_t INPUT_DRV_TASK_PRIORITY   = tskIDLE_PRIORITY + 2U;
const static uint8_t SOUND_DRV_TASK_PRIORITY   = tskIDLE_PRIORITY + 3U;
const static uint8_t UI_TASK_PRIORITY          = tskIDLE_PRIORITY + 4U;
// *****************************************************************************

// Timer Task priority should be high. Otherwise if some task with highest
// priority will take over for long enough period, timer task wont be able to
// call timer callback for AppTask. If AppTask will not receive timer message
// within two times of timer period it will error out(queue empty error).
#if (configTIMER_TASK_PRIORITY != (configMAX_PRIORITIES - 1u))
  #warning "When Timer Task priority isn't highest"
#endif

// *****************************************************************************
// ***   Display Configuration   ***********************************************
// *****************************************************************************

// Max line in pixels for allocate buffer in Display Driver. Usually equal to
// maximum number of pixels in line. But sometimes can be greater than that.
// For example ILI9488 uses 18 bit color(3 bytes per pixel) and if 16 bit color 
// is used(2 bytes per pixel) in order to prepare data display driver need 1.5
// times more memory
static constexpr uint32_t DISPLAY_MAX_BUF_LEN = 480 * 1.5;

// By default there only one update area, that merges all update requests
// by making multiple areas, there can be multiple non-intersect areas(intersect
// areas still will be merged into one).
#define MULTIPLE_UPDATE_AREAS 32

// Color depth used by display
#define COLOR_24BIT
//#define COLOR_16BIT
//#define COLOR_3BIT

#if defined(COLOR_24BIT)
// ***   Color type define   ***************************************************
typedef uint32_t color_t;
// ***   Color definitions   ***************************************************
enum Color
{
  COLOR_BLACK           = 0x00000000, //   0,   0,   0
  COLOR_VERYDARKGREY    = 0x00202020, //  32,  32,  32
  COLOR_DARKGREY        = 0x00404040, //  64,  64,  64
  COLOR_GREY            = 0x00808080, // 128, 128, 128
  COLOR_LIGHTGREY       = 0x00C0C0C0, // 192, 192, 192
  COLOR_WHITE           = 0x00FFFFFF, // 255, 255, 255

  COLOR_VERYDARKRED     = 0x00000020, //   0,   0,  32
  COLOR_DARKRED         = 0x00000040, //   0,   0,  64
  COLOR_MEDIUMRED       = 0x00000080, //   0,   0, 128
  COLOR_LIGHTRED        = 0x000000C0, //   0,   0, 192
  COLOR_RED             = 0x000000FF, //   0,   0, 255

  COLOR_VERYDARKGREEN   = 0x00002000, //   0,  32,   0
  COLOR_DARKGREEN       = 0x00004000, //   0,  64,   0
  COLOR_MEDIUMGREEN     = 0x00008000, //   0, 128,   0
  COLOR_LIGHTGREEN      = 0x0000C000, //   0, 192,   0
  COLOR_GREEN           = 0x0000FF00, //   0, 255,   0

  COLOR_VERYDARKBLUE    = 0x00200000, //  32,   0,   0
  COLOR_DARKBLUE        = 0x00400000, //  64,   0,   0
  COLOR_MEDIUMBLUE      = 0x00800000, // 128,   0,   0
  COLOR_LIGHTBLUE       = 0x00C00000, // 192,   0,   0
  COLOR_BLUE            = 0x00FF0000, // 255,   0,   0

  COLOR_VERYDARKYELLOW  = 0x00002020, //  32,  32,   0
  COLOR_DARKYELLOW      = 0x00004040, //  64,  64,   0
  COLOR_MEDIUMYELLOW    = 0x00008080, // 128, 128,   0
  COLOR_LIGHTYELLOW     = 0x0000C0C0, // 192, 192,   0
  COLOR_YELLOW          = 0x0000FFFF, // 255, 255,   0

  COLOR_VERYDARKCYAN    = 0x00202000, //   0,  32,  32
  COLOR_DARKCYAN        = 0x00404000, //   0,  64,  64
  COLOR_MEDIUMCYAN      = 0x00808000, //   0, 128, 128
  COLOR_LIGHTCYAN       = 0x00C0C000, //   0, 192, 192
  COLOR_CYAN            = 0x00FFFF00, //   0, 255, 255

  COLOR_VERYDARKMAGENTA = 0x00200020, //  32,   0,  32
  COLOR_DARKMAGENTA     = 0x00400040, //  64,   0,  64
  COLOR_MEDIUMMAGENTA   = 0x00800080, // 128,   0, 128
  COLOR_LIGHTMAGENTA    = 0x00C000C0, // 192,   0, 192
  COLOR_MAGENTA         = 0x00FF00FF, // 255,   0, 255
};
#elif defined(COLOR_16BIT)
// ***   Color type define   ***************************************************
typedef uint16_t color_t;
// ***   Color definitions   ***************************************************
enum Color
{
  COLOR_BLACK           = 0x0000, //   0,   0,   0
  COLOR_VERYDARKGREY    = 0xEF7B, //  32,  32,  32
  COLOR_DARKGREY        = 0xEF7B, //  64,  64,  64
  COLOR_GREY            = 0xEF7B, // 128, 128, 128
  COLOR_LIGHTGREY       = 0x18C6, // 192, 192, 192
  COLOR_WHITE           = 0xFFFF, // 255, 255, 255

  COLOR_VERYDARKRED     = 0x0018, //  32,   0,   0
  COLOR_DARKRED         = 0x0038, //  64,   0,   0
  COLOR_MEDIUMRED       = 0x0078, // 128,   0,   0
  COLOR_LIGHTRED        = 0x00B8, // 192,   0,   0
  COLOR_RED             = 0x00F8, // 255,   0,   0

  COLOR_VERYDARKGREEN   = 0xE000, //   0,  32,   0
  COLOR_DARKGREEN       = 0xE001, //   0,  64,   0
  COLOR_MEDIUMGREEN     = 0xE003, //   0, 128,   0
  COLOR_LIGHTGREEN      = 0xE005, //   0, 192,   0
  COLOR_GREEN           = 0xE007, //   0, 255,   0

  COLOR_VERYDARKBLUE    = 0x0300, //   0,   0,  32
  COLOR_DARKBLUE        = 0x0700, //   0,   0,  64
  COLOR_MEDIUMBLUE      = 0x0F00, //   0,   0, 128
  COLOR_LIGHTBLUE       = 0x1700, //   0,   0, 192
  COLOR_BLUE            = 0x1F00, //   0,   0, 255

  COLOR_VERYDARKYELLOW  = 0xE018, //  32,  32,   0
  COLOR_DARKYELLOW      = 0xE039, //  64,  64,   0
  COLOR_MEDIUMYELLOW    = 0xE07B, // 128, 128,   0
  COLOR_LIGHTYELLOW     = 0xE0BD, // 192, 192,   0
  COLOR_YELLOW          = 0xE0FF, // 255, 255,   0

  COLOR_VERYDARKCYAN    = 0xE300, //   0,  32,  32
  COLOR_DARKCYAN        = 0xE701, //   0,  64,  64
  COLOR_MEDIUMCYAN      = 0xEF03, //   0, 128, 128
  COLOR_LIGHTCYAN       = 0xF705, //   0, 192, 192
  COLOR_CYAN            = 0xFF07, //   0, 255, 255

  COLOR_VERYDARKMAGENTA = 0x0318, //  32,   0,  32
  COLOR_DARKMAGENTA     = 0x0738, //  64,   0,  64
  COLOR_MEDIUMMAGENTA   = 0x0F78, // 128,   0, 128
  COLOR_LIGHTMAGENTA    = 0x17B8, // 192,   0, 192
  COLOR_MAGENTA         = 0x1FF8, // 255,   0, 255
};
#elif defined(COLOR_3BIT)
// ***   Color type define   ***************************************************
typedef uint8_t color_t;
// ***   Color definitions   ***************************************************
enum Color
{
  COLOR_BLACK           = 0x00, // 0, 0, 0
  COLOR_RED             = 0x04, // 1, 0, 0
  COLOR_YELLOW          = 0x05, // 1, 0, 1
  COLOR_GREEN           = 0x02, // 0, 1, 0
  COLOR_CYAN            = 0x06, // 0, 0, 0
  COLOR_BLUE            = 0x01, // 0, 0, 1
  COLOR_MAGENTA         = 0x03, // 0, 1, 0
  COLOR_WHITE           = 0x07, // 1, 1, 1

  // Color mapped to basic colors
  COLOR_DARKGREY = COLOR_BLACK,
  COLOR_GREY = COLOR_WHITE
};
#else
  #error NO COLOR DEPTH DEFINED
#endif

// *****************************************************************************
// ***   Macroses   ************************************************************
// *****************************************************************************

// Number of array elements
#define NumberOf(x) (sizeof(x)/sizeof((x)[0]))

// MIN and MAX
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// Break macro - useful for debugging
#define Break() asm volatile("bkpt #0")

// *****************************************************************************
// ***   Overloaded operators   ************************************************
// *****************************************************************************
void* operator new(size_t sz);
void* operator new[](size_t sz);
void operator delete(void* p);
void operator delete[](void* p);
void* operator new(size_t size, void* p);
void* operator new[](size_t size, void* p);
void operator delete(void*, void*);
void operator delete[](void*, void*);

#endif
