//******************************************************************************
//  @file DevCfg.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: Config file, header
//
//  This file contains user configuration for DevCore library.
//  It will be included in DefCfg.h file
//
//******************************************************************************

#ifndef DevCfgUsr_h
#define DevCfgUsr_h

// *****************************************************************************
// ***   Include for hardware   ************************************************
// *****************************************************************************
#include "stm32f4xx.h"

// *****************************************************************************
// ***   RTOS Wrapper   ********************************************************
// *****************************************************************************
#define FREERTOS_WRAPPER

// *****************************************************************************
// ***   Tasks stack size and priorities configuration   ***********************
// *****************************************************************************

// *** Input task priority & stack size   **************************************
#define INPUT_DRV_TASK_PRIORITY (RTOS_IDLE_TASK_PRIORITY + 2u)
#define INPUT_DRV_TASK_STACK_SIZE RTOS_MINIMAL_STACK_SIZE
// *** Application task priority & stack size   ********************************
#define APPLICATION_TASK_PRIORITY (RTOS_IDLE_TASK_PRIORITY + 3u)
#define APPLICATION_TASK_STACK_SIZE 1536u

// *****************************************************************************
// ***   Display Configuration   ***********************************************
// *****************************************************************************

// Max line in pixels for allocate buffer in Display Driver. Usually equal to
// maximum number of pixels in line. But sometimes can be greater than that.
// For example ILI9488 uses 18 bit color(3 bytes per pixel) and if 16 bit color
// is used(2 bytes per pixel) in order to prepare data display driver need 1.5
// times more memory
#define DISPLAY_MAX_BUF_LEN ((uint32_t)(480 * 1.5))

// Color depth used by display
#define COLOR_24BIT

// By uncommenting this line, display task will update only specific area than
// have to be updated
#define UPDATE_AREA_ENABLED

// In some cases one area isn't enough. In case of changes in small areas far
// away on a large display, it makes sense to have multiple areas that can be
// updated this option defines how many such areas is present. In case of
// overflow, code will merge areas to update everything that have to be updated.
#define MULTIPLE_UPDATE_AREAS 32u

// Display FPS/Touch/Update Area debug options
//#define DISPLAY_DEBUG_INFO
//#define DISPLAY_DEBUG_AREA
//#define DISPLAY_DEBUG_TOUCH

// *****************************************************************************
// ***   Break() macro   *******************************************************
// *****************************************************************************

// Break macro: in case something goes wrong(like error happen), DevCore uses
// Break() call. This macro should be defined either as break command for
// debugging purposes, or as assert.
#define Break() asm volatile("bkpt #0")

#endif
