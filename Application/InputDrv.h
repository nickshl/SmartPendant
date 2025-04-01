//******************************************************************************
//  @file InputDrv.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: Input Driver Class, header
//
//  @section LICENSE
//
//   Software License Agreement (Modified BSD License)
//
//   Copyright (c) 2023, Devtronic & Nicolai Shlapunov
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

#ifndef InputDrv_h
#define InputDrv_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "AppTask.h"

// *****************************************************************************
// * Input Driver Class. This class implement work with user input elements like 
// * buttons and encoders.
class InputDrv : public AppTask
{
  public:
    // *************************************************************************
    // ***   Enum with all buttons   *******************************************
    // *************************************************************************
    typedef enum : uint8_t
    {
      BTN_LEFT = 0u,  // Face Left button
      BTN_RIGHT,      // Face Right button
      BTN_LEFT_UP,    // Side Left Up button
      BTN_LEFT_DOWN,  // Side Left Down button
      BTN_RIGHT_UP,   // Side Right Up button
      BTN_RIGHT_DOWN, // Side Right Down button
      BTN_USR,        // Key button on BlackPill
      BTN_MAX         // Buttons count
    } ButtonType;

    // ***   Enum with masks for all buttons   *********************************
    typedef enum : uint8_t
    {
      BTNM_EMPTY = 0u,
      BTNM_LEFT =       (1u << BTN_LEFT),       // Face Left button
      BTNM_RIGHT =      (1u << BTN_RIGHT),      // Face Right button
      BTNM_LEFT_UP =    (1u << BTN_LEFT_UP),    // Side Left Up button
      BTNM_LEFT_DOWN =  (1u << BTN_LEFT_DOWN),  // Side Left Down button
      BTNM_RIGHT_UP =   (1u << BTN_RIGHT_UP),   // Side Right Up button
      BTNM_RIGHT_DOWN = (1u << BTN_RIGHT_DOWN), // Side Right Down button
      BTNM_USR =        (1u << BTN_USR),        // Key button on BlackPill
    } ButtonMaskType;

    // *************************************************************************
    // ***   Structure to describe callback   **********************************
    // *************************************************************************
    typedef struct CallbackListEntryStruct
    {
      private:
        AppTask* callback_task = nullptr;
        CallbackPtr callback = nullptr;
        void* obj_ptr = nullptr;
        uint8_t mask = 0u; // Mask for button callback list to handle particular button
        struct CallbackListEntryStruct* next = nullptr;
        struct CallbackListEntryStruct* prev = nullptr;
        // InputDrv is friend of structure for access to pointers
        friend class InputDrv;
    } CallbackListEntry;

    // *************************************************************************
    // ***   Button structure for callback   ***********************************
    // *************************************************************************
    typedef struct
    {
      ButtonType btn;
      bool state;
    } ButtonCallbackData;

    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    // * This class is singleton. For use this class you must call GetInstance()
    // * to receive reference to Input Driver class
    static InputDrv& GetInstance(void);

    // *************************************************************************
    // ***   Public: Init Input Driver Task   **********************************
    // *************************************************************************
    void InitTask(TIM_HandleTypeDef& htm, uint32_t ch);

    // *************************************************************************
    // ***   Input Driver Setup   **********************************************
    // *************************************************************************
    virtual Result Setup();

    // *************************************************************************
    // ***   Input Driver Loop   ***********************************************
    // *************************************************************************
    // * If FreeRTOS task used, this function just call ProcessInput() with 1 ms
    // * period. If FreeRTOS tick is 1 ms - this task must have highest priority 
    virtual Result Loop();

    // *************************************************************************
    // ***   Public: Add Buttons Callback handler   ****************************
    // *************************************************************************
    void AddButtonsCallbackHandler(AppTask* callback_task, CallbackPtr callback, void* obj_ptr, uint8_t mask, CallbackListEntry& cble);

    // *************************************************************************
    // ***   Public: Delete Buttons Callback handler   *************************
    // *************************************************************************
    void DeleteButtonsCallbackHandler(CallbackListEntry& cble);

    // *************************************************************************
    // ***   Public: Add Encoder Callback handler   ****************************
    // *************************************************************************
    void AddEncoderCallbackHandler(AppTask* callback_task, CallbackPtr callback, void* obj_ptr, CallbackListEntry& cble);

    // *************************************************************************
    // ***   Public: Delete Encoder Callback handler   *************************
    // *************************************************************************
    void DeleteEncoderCallbackHandler(CallbackListEntry& cble);

    // *************************************************************************
    // ***   Get button current state   ****************************************
    // *************************************************************************
    bool GetButtonCurrentState(ButtonType button);

    // *************************************************************************
    // ***   Get button state   ************************************************
    // *************************************************************************
    // Return button state: true - pressed, false - unpressed
    bool GetButtonState(ButtonType button);

    // *************************************************************************
    // ***   Get button state   ************************************************
    // *************************************************************************
    // Return button state change flag: true - changed, false - not changed
    bool GetButtonState(ButtonType button, bool& btn_state);

    // *************************************************************************
    // ***   Get encoder counts from last call   *******************************
    // *************************************************************************
    // * Return state of encoder. Class counts encoder clicks and stored inside.
    // * This function substract from current encoder counter last_enc_val and
    // * return it to user. Before return last_enc_val will be assigned to
    // * current encoder counter.
    int32_t GetEncoderState(int32_t& last_enc_val);

    // *************************************************************************
    // ***   Get Encoder speed   ***********************************************
    // *************************************************************************
    uint32_t GetEncoderSpeed() {return enc_speed;}

  private:
    // How many cycles button must change state before state will be changed in
    // result returned by GetButtonState() function. For reduce debouncing
    const static uint32_t BUTTON_READ_DELAY = 8u;

    // Handle to timer used for process encoders input
    TIM_HandleTypeDef* htim = nullptr;
    // Channel that used
    uint32_t channel = 0u;

    // Button handlers
    CallbackListEntry* btn_callback_list = nullptr;
    // Encoder handlers
    CallbackListEntry* enc_callback_list = nullptr;

    // Ticks variable
    uint32_t last_wake_ticks = 0u;

    // Last button states for GetButtonState() function. Can be called only from one place!
    bool buttons_last_state[BTN_MAX] = {0};

    // Last value for reduce overhead in user task
    int32_t last_enc_value = 0;
    uint32_t last_enc_ms = 0u;
    uint32_t enc_speed = 0u;

    // *************************************************************************
    // ***   Structure to describe button   ************************************
    // *************************************************************************
    typedef struct
    {
      GPIO_TypeDef* button_port;// Button port
      uint16_t button_pin;      // Button pin
      GPIO_PinState pin_state;  // High/low on input treated as pressed
      uint8_t btn_state_cnt;    // Counter for reduce debouncing
      bool btn_state;           // Button state returned by GetButtonState() function
      bool btn_state_tmp;       // Temporary button state for reduce debouncing
    } ButtonProfile;

    // ***   Structures array for describe buttons inputs  *********************
    ButtonProfile buttons[BTN_MAX] =
    {
      {BTN_LEFT_GPIO_Port,  BTN_LEFT_Pin,  GPIO_PIN_RESET, 0, false, false},
      {BTN_RIGHT_GPIO_Port, BTN_RIGHT_Pin, GPIO_PIN_RESET, 0, false, false},
      {BTN_LU_GPIO_Port,    BTN_LU_Pin,    GPIO_PIN_RESET, 0, false, false},
      {BTN_LD_GPIO_Port,    BTN_LD_Pin,    GPIO_PIN_RESET, 0, false, false},
      {BTN_RU_GPIO_Port,    BTN_RU_Pin,    GPIO_PIN_RESET, 0, false, false},
      {BTN_RD_GPIO_Port,    BTN_RD_Pin,    GPIO_PIN_RESET, 0, false, false},
      {BTN_USR_GPIO_Port,   BTN_USR_Pin,   GPIO_PIN_RESET, 0, false, false}
    };

    // *************************************************************************
    // ***   Button data for callback   ****************************************
    // *************************************************************************
    static ButtonCallbackData btn_callback_data[BTN_MAX][2u];

    // *************************************************************************
    // ***   Process Button Input function   ***********************************
    // *************************************************************************
    bool ProcessButtonInput(ButtonProfile& button);

    // *************************************************************************
    // ** Private constructor. Only GetInstance() allow to access this class. **
    // *************************************************************************
    InputDrv() : AppTask(INPUT_DRV_TASK_STACK_SIZE, INPUT_DRV_TASK_PRIORITY, "InputDrv") {};
};

#endif
