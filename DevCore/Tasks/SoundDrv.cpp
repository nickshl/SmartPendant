//******************************************************************************
//  @file SoundDrv.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: Sound Driver Class, implementation
//
//  @copyright Copyright (c) 2017, Devtronic & Nicolai Shlapunov
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
#include "SoundDrv.h"

#if defined(SOUNDDRV_ENABLED)

#include "Rtos.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
SoundDrv& SoundDrv::GetInstance(void)
{
  // This class is static and declared here
  static SoundDrv sound_drv;
  // Return reference to class
  return sound_drv;
}

// *****************************************************************************
// ***   Init Display Driver Task   ********************************************
// *****************************************************************************
void SoundDrv::InitTask(TIM_HandleTypeDef* htm)
{
  // Save timer handle
  htim = htm;
  // Create task
  CreateTask();
}

// *****************************************************************************
// ***   Sound Driver Setup   **************************************************
// *****************************************************************************
Result SoundDrv::Setup()
{
  // Init ticks variable
  last_wake_ticks = RtosTick::GetTickCount();
  // Always ok
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Sound Driver Loop   ***************************************************
// *****************************************************************************
Result SoundDrv::Loop()
{
  // Flag
  bool is_playing = false;
  // Take mutex before start playing sound
  melody_mutex.Lock();
  // Delay for playing one frequency
  uint32_t current_delay_ms = delay_ms;
  // If no current melody or melody size is zero - skip playing
  if((sound_table != nullptr) && (sound_table_size != 0U))
  {
    // Set flag that still playing sound
    is_playing = true;
    // If frequency greater than 18 Hz
    if(((uint32_t)sound_table[sound_table_position] >> 4U) > 0x12U)
    {
      Tone(sound_table[sound_table_position] >> 4U);
    }
    else
    {
      // Otherwise "play" silence
      Tone(0U);
    }

    // Init ticks variable
    if(sound_table_position == 0U)
    {
      last_wake_ticks = RtosTick::GetTickCount();
    }
    // Get retry counter from table and calculate delay
    current_delay_ms *= sound_table[sound_table_position] & 0x0FU;

    // Increase array index
    sound_table_position++;
    // If end of melody reached
    if(sound_table_position >= sound_table_size)
    {
      // If set repeat flag
      if(repeat == true)
      {
        // Reset index for play melody from beginning
        sound_table_position = 0U;
      }
      else
      {
        // Otherwise stop playing sound
        is_playing = false;
      }
    }
  }
  // Give mutex after start playing sound
  melody_mutex.Release();

  // Pause until next tick
  RtosTick::DelayUntilMs(last_wake_ticks, current_delay_ms);

  // Using semaphore here helps block this task while task wait request for
  // sound playing.
  if(is_playing == false)
  {
    // Stop playing sound before wait semaphore
    StopSound();
    // Wait semaphore for start play melody
    sound_update.Take();
  }

  // Always run
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***   Click function   ******************************************************
// *****************************************************************************
void SoundDrv::Click()
{
  if((mute == false) && (sound_table != nullptr))
  {
    // Set Speaker output pin for click
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
    // Read when actual state changed
    while(HAL_GPIO_ReadPin(BUZZER_GPIO_Port, BUZZER_Pin) == GPIO_PIN_RESET);
    // Clear Speaker output pin
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
  }
}

// *****************************************************************************
// ***   Beep function   *******************************************************
// *****************************************************************************
void SoundDrv::Beep(uint16_t freq, uint16_t del, bool pause_after_play)
{
  // Take mutex before beeping - prevent play melody during beeping.
  melody_mutex.Lock();
  // Start play tone
  Tone(freq);
  // Delay
  RtosTick::DelayMs(del);
  // Stop play tone
  Tone(0);
  // If flag is set
  if(pause_after_play == true)
  {
    // Delay with same value as played sound
    RtosTick::DelayMs(del);
  }
  // Give mutex after beeping
  melody_mutex.Release();
}

// *****************************************************************************
// ***   Play sound function   *************************************************
// *****************************************************************************
void SoundDrv::PlaySound(const uint16_t* melody, uint16_t size, uint16_t temp_ms, bool rep)
{
  // Parameters: pointer to melody table, size of melody and repetition flag.
  // Format of sounds: 0x***#, where *** frequency, # - delay in temp_ms intervals

  // If pointer is not nullptr, if size & freq time greater than zero
  if((melody != nullptr) && (size > 0U) && (temp_ms > 0U))
  {
    // If already playing any melody
    if(IsSoundPlayed() == true)
    {
      // Stop it first
      StopSound();
    }

    // Take mutex before start playing melody
    melody_mutex.Lock();
    // Set repeat flag for melody
    repeat = rep;
    // Set time for one frequency
    delay_ms = temp_ms;
    // Set initial index for melody
    sound_table_position = 0;
    // Set melody size
    sound_table_size = size;
    // Set melody pointer
    sound_table = melody;
    // Give mutex after start playing melody
    melody_mutex.Release();

    // Give semaphore for start play melody
    sound_update.Give();
  }
}

// *****************************************************************************
// ***   Stop sound function   *************************************************
// *****************************************************************************
void SoundDrv::StopSound(void)
{
  // Take mutex before stop playing sound
  melody_mutex.Lock();
  // Clear sound table pointer
  sound_table = nullptr;
  // Clear sound table size
  sound_table_size = 0;
  // Clear sound table index
  sound_table_position = 0;
  // Set time for one frequency
  delay_ms = 100U;
  // Set repeat flag for melody
  repeat = false;
  // Stop sound
  Tone(0);
  // Give mutex after stop playing sound
  melody_mutex.Release();
}

// *****************************************************************************
// ***   Mute sound function   *************************************************
// *****************************************************************************
void SoundDrv::Mute(bool mute_flag)
{
  // Set mute flag
  mute = mute_flag;
  // If mute flag is set - call Tone() for stop tone
  if(mute == true)
  {
    Tone(0U);
  }
}

// *****************************************************************************
// ***   Is sound played function   ********************************************
// *****************************************************************************
bool SoundDrv::IsSoundPlayed(void)
{
  // Return variable, false by default
  bool ret = false;
  // If sound_table is not nullptr - we still playing melody. No sense to use
  // mutex here - get pointer is atomic operation.
  if(sound_table != nullptr)
  {
    ret = true;
  }
  // Return result
  return ret;
}

// *****************************************************************************
// ***   Process Button Input function   ***************************************
// *****************************************************************************
void SoundDrv::Tone(uint16_t freq)
{
  // FIX ME: rewrite comment
  // Òàéìåð çàïóñêàåòñÿ ñ ïàðàìåòðàìè:
  // Clock source: System Clock
  // Mode: CTC top = OCR2
  // OC2 output: Toggle on compare match
  // È ñ ðàçíûìè äåëèòåëÿìè äëÿ ðàçíûõ ÷àñòîò, ïîòîìó êàê:
  // ïðè äåëèòåëå 64 íåâîçìîæíî ïîëó÷èòü ÷àñòîòó íèæå ~750Ãö
  // ïðè äåëèòåëå 256 íà ÷àñòîòàõ > ~1500Ãö âûñîêà ïîãðåøíîñòü ãåíåðàöèè
  // Äåëåíèå íà 4 àðãóìåíòîâ äëÿ òîãî, ÷òî áû AVR_Clock_Freq/x íå ïðåâûñèëî word
  // Äåëåíèå íà äâà â êîíöå, ïîòîìó êàê íóæåí ïîëóïåðèîä
  // Åñëè çâóê îòêëþ÷åí - òàéìåð îñòàíàâëèâàåòñÿ è ñíèìàåòñÿ íàïðÿæåíèå ñ ïèùàëêè
  if((freq > 11) && (mute == false))
  {
    // Calculate prescaler
    uint32_t prescaler = (HAL_RCC_GetHCLKFreq()/100U) / freq;
    // Set the Prescaler value
    htim->Instance->PSC = (uint32_t)prescaler;
    // Generate an update event to reload the Prescaler and the repetition
    // counter(only for TIM1 and TIM8) value immediately
    htim->Instance->EGR = TIM_EGR_UG;
    // Start timer in Output Compare match mode
    (void) HAL_TIM_OC_Start(htim, channel);
  }
  else
  {
    // Stop timer
    HAL_TIM_OC_Stop(htim, channel);
    // Clear Speaker output pin for decrease power consumer
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
  }
}

#endif
