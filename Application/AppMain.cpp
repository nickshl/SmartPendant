//******************************************************************************
//  @file AppMain.cpp
//  @author Nicolai Shlapunov
//
//  @details Application: Main file, header
//
//  @copyright Copyright (c) 2023, Devtronic & Nicolai Shlapunov
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
#include "DevCfg.h"
// Objects
#include "StHalSpi.h"
#include "StHalIicThreadSafe.h"
#include "StHalUart.h"
#include "StHalGpio.h"
#include "ILI9488.h"
#include "FT6236.h"
#include "Eeprom24.h"
// NVM
#include "NVM.h"
// Tasks
#include "DisplayDrv.h"
#include "InputDrv.h"
// Application
#include "Application.h"
#include "GrblComm.h"
#include "GraphDemo.h"
#include "Tetris.h"

// *****************************************************************************
// ***   Objects   *************************************************************
// *****************************************************************************

// GPIOs
static StHalGpio display_cs(LCD_CS_GPIO_Port, LCD_CS_Pin, IGpio::OUTPUT);
static StHalGpio display_dc(LCD_DC_GPIO_Port, LCD_DC_Pin, IGpio::OUTPUT);
static StHalGpio display_rst(LCD_RST_GPIO_Port, LCD_RST_Pin, IGpio::OUTPUT);
// Buttons
static StHalGpio btn_lu(BTN_LU_GPIO_Port, BTN_LU_Pin, IGpio::INPUT);
static StHalGpio btn_ld(BTN_LD_GPIO_Port, BTN_LD_Pin, IGpio::INPUT);
// Interfaces
static StHalSpi spi1(hspi1);
static StHalIicThreadSafe iic1(hi2c1);
static StHalUart uart1(huart1);
// Display & touch
static ILI9488 display(480, 320, spi1, display_cs, display_dc, &display_rst);
static FT6236 touch(iic1, ITouchscreen::ROTATION_LEFT, 320u, 480u);
// NVM: MB85RC256V, no write protection, size 32kB, no pages, but 64 used since we allocate buffer for it
static Eeprom24 eep(iic1, nullptr, 0x8000u, 64u);

// *****************************************************************************
// ***   Main function   *******************************************************
// *****************************************************************************
extern "C" void AppMain(void)
{
  // Init NVM
  NVM::GetInstance().Init(eep);
  // Init Display Driver Task
  DisplayDrv::GetInstance().SetDisplayDrv(&display);
  DisplayDrv::GetInstance().SetTouchDrv(&touch);
  DisplayDrv::GetInstance().InitTask();
  // Init sound task
  SoundDrv::GetInstance().InitTask();
  // Init input task
  InputDrv::GetInstance().SetEncoderTim(&htim2, TIM_CHANNEL_1);
  InputDrv::GetInstance().InitTask();
  // Check Startup conditions
  if(btn_lu.IsLow())
  {
    // If left up button pressed - run Tetris
    Tetris::GetInstance().InitTask();
  }
  else if(btn_ld.IsLow())
  {
    // If left down button pressed - run GraphDemo
    GraphDemo::GetInstance().InitTask();
  }
  else
  {
    // Init GRBL Communication task
    GrblComm::GetInstance().SetUartDrv(uart1);
    GrblComm::GetInstance().InitTask();
    // Init Application Task
    Application::GetInstance().InitTask();
  }
}

// *****************************************************************************
// ***   Stack overflow hook function   ****************************************
// *****************************************************************************
extern "C" void vApplicationStackOverflowHook(TaskHandle_t* px_task, signed portCHAR* pc_task_name)
{
  Break();
  while(1);
}

// *****************************************************************************
// ***   Malloc failed hook function   *****************************************
// *****************************************************************************
extern "C" void vApplicationMallocFailedHook(void)
{
// Commented out since ProgramSender rely on nullptr retunr from new operator
// in case if program is too big.
//  Break();
//  while(1);
}
