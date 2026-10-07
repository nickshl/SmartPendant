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
#include "DevCore.h"
// NVM
#include "NVM.h"
// Tasks
#include "InputDrv.h"
// Application
#include "Application.h"
#include "GrblComm.h"
#include "FramedUart.h"
#include "Tetris.h"

// Hardware
#include "tim.h"

// *****************************************************************************
// ***   Functions   ***********************************************************
// *****************************************************************************

// Detect which crystal can configure clock value
void DetectCrystalValue();
// Jump to internal bootloader
void Bootloader();
// Function to get calibration value for HSI
uint32_t CalibrateHSI();
// Function that returns the clock value captured by TIM11
uint32_t CaptureClockValue();
// Configure PLL with specified source and divider value
void ConfigurePll(uint32_t source, uint32_t pllm);

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
static StHalGpio btn_usr(BTN_USR_GPIO_Port, BTN_USR_Pin, IGpio::INPUT);
// Buzzer
static StHalPwm buzzer_pwm(htim1, TIM_CHANNEL_1);
static StHalGpio buzzer_pin(BUZZER_GPIO_Port, BUZZER_Pin, IGpio::OUTPUT);
// Interfaces
static StHalSpi spi1(hspi1);
static StHalIicThreadSafe iic1(hi2c1);
static StHalUart uart(huart1);
// Display & touch
static ILI9488 display(480, 320, spi1, display_cs, display_dc, &display_rst);
static FT6236 touch(iic1, ITouchscreen::ROTATION_LEFT, 320u, 480u);
// NVM: 24xx256 class I2C EEPROM, no write protection, 32 kB, 64 byte pages.
// Early boards used MB85RC256V FRAM; it was replaced with EEPROM to cut cost,
// so the endurance and timing characteristics are completely different:
// ~1,000,000 erase/write cycles PER PAGE and a 5 ms self-timed write cycle,
// where FRAM was effectively unlimited and instant. Keep write frequency in
// mind before persisting anything that changes at machine rate.
static Eeprom24 eep(iic1, nullptr, 0x8000u, 64u);

// Framed UART transport layer(not static to allow SettingScr access via extern declaration)
FramedUart framed_uart(uart);
// Transport actually in use - the setting takes effect on reboot only
bool framed_transport_in_use = false;

// *****************************************************************************
// ***   Main function   *******************************************************
// *****************************************************************************
extern "C" void AppMain(void)
{
  // Detect 25 or 8 MHz crystal is used
  DetectCrystalValue();
  // Check top edge button state and if it pressed - jump to internal bootloader
  if(btn_usr.IsLow()) Bootloader();

  // Init NVM
  NVM::GetInstance().Init(eep);
  // Init Display Driver Task
  DisplayDrv::GetInstance().InitTask(display, touch);
  // Init sound task
  SoundDrv::GetInstance().InitTask(buzzer_pwm);
  // Init input task
  InputDrv::GetInstance().InitTask(htim2, TIM_CHANNEL_1);
  // Check Startup conditions
  if(btn_lu.IsLow())
  {
    // If left up button pressed - run Tetris
    Tetris::GetInstance().InitTask();
  }
  else
  {
    // Read NVM data to get UART mode
    NVM::GetInstance().ReadData();
    // Update UART speed(via framed_uart to recalculate timeout for framed mode)
    framed_uart.SetBaudRate(NVM::GetInstance().GetValue(NVM::BAUD_RATE));
    // Transmissions per frame before the link is declared down. Has to
    // match the controller's "frame attempts" setting - see SetAttempts().
    framed_uart.SetAttempts(NVM::GetInstance().GetValue(NVM::FRAME_ATTEMPTS));
    // Floor under the baud derived acknowledge timeout, 0 = derived value
    framed_uart.SetAckTimeoutFloor(NVM::GetInstance().GetValue(NVM::ACK_MIN_MS));
    // Find which uart class should be used - plain or framed
    framed_transport_in_use = (NVM::GetInstance().GetValue(NVM::TRANSPORT) != 0);
    // Init GRBL Communication task with appropriate uart handle
    GrblComm::GetInstance().InitTask(framed_transport_in_use ? (IUart&)framed_uart : (IUart&)uart);
    // Init Application Task
    Application::GetInstance().InitTask();
  }
}

// *****************************************************************************
// ***   DetectCrystalValue() function   ***************************************
// *****************************************************************************
void DetectCrystalValue()
{
  // PLLM divider for 25 MHz crystal oscillator by default
  uint32_t pllm = 25u;

  // Configure the main PLL: HSI 16MHz
  ConfigurePll(RCC_PLLSOURCE_HSI, 16u);

  // If measured count is more than 1.5 times more than expected, then 8MHz
  // crystal oscillator is used.
  if(CaptureClockValue() > (SystemCoreClock * 3u / 2u / 1000000u))
  {
    // Set PLLN to 8 for 8MHz crystal oscillator
    pllm = 8u;
    // Activation RTC clock as HSE / 8 which should give 1 MHz with 8 MHz Crystal Oscillator
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV8;
    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) Error_Handler();
    // RTC clock enable
    __HAL_RCC_RTC_ENABLE();
  }

  // Configure the main PLL: source HSE, PLLM divider 25 or 8.
  ConfigurePll(RCC_PLLSOURCE_HSE, pllm);
}

// *****************************************************************************
// ***   Bootloader() function   ***********************************************
// *****************************************************************************
void Bootloader()
{
  //  Calibrate HSI and get calibration value
  uint32_t hsi_calibration_value = CalibrateHSI();

  // Disable peripherals
  HAL_RCC_DeInit();
  HAL_DeInit();
  // Adjusts the Internal High Speed oscillator (HSI) calibration value after deinitialization
  __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(hsi_calibration_value);
  // Disable SysTick timer and reset it to default values
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;

  // Set system memory address
  volatile uint32_t addr = 0x1FFF0000u;
  // Set jump memory location for system memory
  void (*SysMemBootJump)(void) = (void (*)(void))(*((uint32_t*)(addr + 4u)));

  // Interrupts have to be enabled, otherwise USB boot wouldn't work
  __enable_irq();

  // Remap system memory to 0x00000000
  __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
  // Set main stack pointer
  __set_MSP(*(uint32_t *)addr);
  // Call function to jump to the bootloader
  SysMemBootJump();
}

// *****************************************************************************
// ***   CalibrateHSI() function   *********************************************
// *****************************************************************************
uint32_t CalibrateHSI()
{
  // Configure the main PLL: HSI 16MHz
  ConfigurePll(RCC_PLLSOURCE_HSI, 16u);

  // HSI calibration value to return
  uint32_t hsi_calibration_value = 0u;
  // TIM11 clocked from AHB2. This clock equal system clock. TIM11 input capture
  // connected to HSE_RTC clock, which should be 1MHz. As result, TIM11 should
  // count SystemCoreClock / 1000000 cycles per RTC clock cycle.
  // Since calibration value is 5 bits, it mean that we need not more than 32
  // iterations to calibrate HSI to match 1MHz RTC clock. In some cases code
  // below can "oscillate" around the target value, so we limit it to 31
  // iterations.
  for(uint32_t i = 0; i < 0x1Fu; i++)
  {
    // Get measurement
    uint32_t count = CaptureClockValue();
    // Get current calibration value
    hsi_calibration_value = (uint32_t)((RCC->CR & RCC_CR_HSITRIM) >> RCC_CR_HSITRIM_Pos);
    // Check result and correct if needed
    if(count > (SystemCoreClock / 1000000u))
    {
      if(hsi_calibration_value > 0u) hsi_calibration_value--;
      else break; // No room for correction anymore
    }
    else if(count < (SystemCoreClock / 1000000u))
    {
      if(hsi_calibration_value < 0x1Fu) hsi_calibration_value++;
      else break; // No room for correction anymore
    }
    else
    {
      break; // Exact match - calibration completed
    }
    // Adjusts the Internal High Speed oscillator (HSI) calibration value
    __HAL_RCC_HSI_CALIBRATIONVALUE_ADJUST(hsi_calibration_value);
  }

  // Return result. This function does not configure PLL to use external crystal
  // since it should be call before calling bootloader and clock will be
  // deinitialized anyway.
  return hsi_calibration_value;
}

// *****************************************************************************
// ***   CaptureClockValue() function   ****************************************
// *****************************************************************************
uint32_t CaptureClockValue()
{
  // Reset counter
  htim11.Instance->CNT = 0;
  // Start input capture
  HAL_TIM_IC_Start(&htim11, TIM_CHANNEL_1);
  // Clear TIM_SR_CC1IF flag that is set at the start by some reason
  (void) HAL_TIM_ReadCapturedValue(&htim11, TIM_CHANNEL_1);
  // Wait until first Capture compare 1 event
  while((htim11.Instance->SR & (TIM_SR_CC1IF)) != (TIM_SR_CC1IF));
  // Save first value
  uint32_t val1 = HAL_TIM_ReadCapturedValue(&htim11, TIM_CHANNEL_1);
  // Wait until second Capture compare 1 event
  while((htim11.Instance->SR & (TIM_SR_CC1IF)) != (TIM_SR_CC1IF));
  // Save second value
  uint32_t val2 = HAL_TIM_ReadCapturedValue(&htim11, TIM_CHANNEL_1);
  // Stop the timer
  HAL_TIM_IC_Stop(&htim11, TIM_CHANNEL_1);

  // Return measured count
  return (val2 - val1);
}

// *****************************************************************************
// ***   ConfigurePll() function   *********************************************
// *****************************************************************************
void ConfigurePll(uint32_t source, uint32_t pllm)
{
  // Wait till HSE is ready
  while(__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET);
  // Switch clock source to HSE
  __HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_HSE);
  // Disable the main PLL
  __HAL_RCC_PLL_DISABLE();
  // Wait till PLL is disabled
  while(__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) != RESET);
  // Set requested PLL source
  RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLSRC) | source;
  // Set requested PLLM
  RCC->PLLCFGR = (RCC->PLLCFGR & ~RCC_PLLCFGR_PLLM) | (pllm << RCC_PLLCFGR_PLLM_Pos);
  // Enable the main PLL
  __HAL_RCC_PLL_ENABLE();
  // Wait till PLL is ready
  while(__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) == RESET);
  // Switch clock source to PLL
  __HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_PLLCLK);
  // Update System clock variable - this function does not work if HSE crystal
  // doesn't match frequency that set in CubeMX, because it relay on HSE_VALUE
  // define, not a variable, as result it can't be updated from the value set in
  // CubeMX to the actual value. But since we reconfigure PLL to match the same
  // system clock as set in *.ioc file, it is not necessary to update clocks
  // anyway.
  // SystemCoreClockUpdate();
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
// Commented out since ProgramSender rely on nullptr return from new operator
// in case if program is too big.
//  Break();
//  while(1);
}
