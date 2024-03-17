//******************************************************************************
//  @file SoundDrv.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: Sound Driver Class, header
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

#ifndef SoundDrv_h
#define SoundDrv_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"

#if defined(SOUNDDRV_ENABLED)

#include "AppTask.h"
#include "RtosMutex.h"
#include "RtosSemaphore.h"

// *****************************************************************************
// ***   Sound Driver Class. This class implement work with sound.   ***********
// *****************************************************************************
class SoundDrv : public AppTask
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    // * This class is singleton. For use this class you must call GetInstance()
    // * to receive reference to Sound Driver class
    static SoundDrv& GetInstance(void);

    // *************************************************************************
    // ***   Init Sound Driver Task   ******************************************
    // *************************************************************************
    virtual void InitTask(TIM_HandleTypeDef *htm = SOUND_HTIM);

    // *************************************************************************
    // ***   Sound Driver Setup   **********************************************
    // *************************************************************************
    virtual Result Setup();

    // *************************************************************************
    // ***   Sound Driver Loop   ***********************************************
    // *************************************************************************
    virtual Result Loop();

    // *************************************************************************
    // ***   Click function   ***************************************************
    // *************************************************************************
    void Click();

    // *************************************************************************
    // ***   Beep function   ***************************************************
    // *************************************************************************
    void Beep(uint16_t freq, uint16_t del, bool pause_after_play = false);

    // *************************************************************************
    // ***   Play sound function   *********************************************
    // *************************************************************************
    void PlaySound(const uint16_t* melody, uint16_t size, uint16_t temp_ms = 100U, bool rep = false);

    // *************************************************************************
    // ***   Stop sound function   *********************************************
    // *************************************************************************
    void StopSound(void);

    // *************************************************************************
    // ***   Mute sound function   *********************************************
    // *************************************************************************
    void Mute(bool mute_flag);

    // *************************************************************************
    // ***   Is sound played function   ****************************************
    // *************************************************************************
    bool IsSoundPlayed(void);

  private:
    // Timer handle
    TIM_HandleTypeDef* htim = SOUND_HTIM;
    // Timer channel
    uint32_t channel = SOUND_CHANNEL;

    // Ticks variable
    uint32_t last_wake_ticks = 0U;

    // Pointer to table contains melody
    const uint16_t* sound_table = nullptr;
    // Size of table
    uint16_t sound_table_size = 0U;
    // Current position
    uint16_t sound_table_position = 0U;
    // Current frequency delay
    uint16_t current_delay = 0U;
    // Time for one frequency in ms
    uint32_t delay_ms = 100U;
    // Repeat flag
    bool repeat = false;

    // Mute flag
    bool mute = false;

    // Mutex to synchronize when playing melody frames
    RtosMutex melody_mutex;

    // Semaphore for start play sound
    RtosSemaphore sound_update;

    // *************************************************************************
    // ***   Process Button Input function   ***********************************
    // *************************************************************************
    void Tone(uint16_t freq);

    // *************************************************************************
    // ** Private constructor. Only GetInstance() allow to access this class. **
    // *************************************************************************
    SoundDrv() : AppTask(SOUND_DRV_TASK_STACK_SIZE, SOUND_DRV_TASK_PRIORITY,
                         "SoundDrv") {};
};

#endif

#endif
