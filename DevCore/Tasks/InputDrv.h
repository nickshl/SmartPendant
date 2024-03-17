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

// *****************************************************************************
// ***   DevCfg Include   ******************************************************
// *****************************************************************************
#include "DevCfg.h" // Need to get INPUTDRV_ENABLED define

#if defined(INPUTDRV_ENABLED)

// It placed there in case user has it own InputDrv class
#ifndef InputDrv_h
#define InputDrv_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
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
    typedef enum 
    {
      EXT_LEFT,  // Left ext port
      EXT_RIGHT, // Right ext port
      EXT_MAX    // Ext port count
    } PortType;

    // *************************************************************************
    // ***   Enum with all devices types   *************************************
    // *************************************************************************
    typedef enum 
    {
      EXT_DEV_NONE, // No device
      EXT_DEV_BTN,  // Buttons(cross)
      EXT_DEV_ENC,  // Encoder
      EXT_DEV_JOY,  // Joystick
      EXT_DEV_MAX   // Device types count
    } ExtDeviceType;
    
    // *************************************************************************
    // ***   Enum with all buttons   *******************************************
    // *************************************************************************
    typedef enum 
    {
      BTN_UP,    // Up button
      BTN_LEFT,  // Left button
      BTN_DOWN,  // Down button
      BTN_RIGHT, // Right button
      BTN_MAX    // Buttons count
    } ButtonType;

    // *************************************************************************
    // ***   Enum with all encoder buttons   ***********************************
    // *************************************************************************
    typedef enum
    {
      ENC_BTN_ENT,   // Press on the knob
      ENC_BTN_BACK,  // Small button
      ENC_BTN_MAX    // Buttons count
    } EncButtonType;

    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    // * This class is singleton. For use this class you must call GetInstance()
    // * to receive reference to Input Driver class
    static InputDrv& GetInstance(void);

    // *************************************************************************
    // ***   Init Input Driver Task   ******************************************
    // *************************************************************************
    // * This function initialize Input Driver class. If htim provided, this 
    // * timer will be used instead FreeRTOS task.
    virtual void InitTask(TIM_HandleTypeDef* htm, ADC_HandleTypeDef* had);

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
    // ***   Process Input function   ******************************************
    // *************************************************************************
    // * Main class function - must call periodically for process user input.
    // * If timer used, this function must be called from interrupt handler.
    void ProcessInput(void);

    // *************************************************************************
    // ***   Process Encoders Input function   *********************************
    // *************************************************************************
    void ProcessEncodersInput(void);

    // *************************************************************************
    // ***   Get device type   *************************************************
    // *************************************************************************
    ExtDeviceType GetDeviceType(PortType port);

    // *************************************************************************
    // ***   Get button state   ************************************************
    // *************************************************************************
    // Return button state: true - pressed, false - unpressed
    bool GetButtonState(PortType port, ButtonType button);
    
    // *************************************************************************
    // ***   Get button state   ************************************************
    // *************************************************************************
    // Return button state change flag: true - changed, false - not changed
    bool GetButtonState(PortType port, ButtonType button, bool& btn_state);

    // *************************************************************************
    // ***   Get encoder counts from last call - CAN BE CALLED FROM ONE TASK   *
    // *************************************************************************
    int32_t GetEncoderState(PortType port);

    // *************************************************************************
    // ***   Get encoder counts from last call   *******************************
    // *************************************************************************
    // * Return state of encoder. Class counts encoder clicks and stored inside.
    // * This function substract from current encoder counter last_enc_val and
    // * return it to user. Before return last_enc_val will be assigned to
    // * current encoder counter.
    int32_t GetEncoderState(PortType port, int32_t& last_enc_val);

    // *************************************************************************
    // ***   Get button state   ************************************************
    // *************************************************************************
    // Return button state: true - pressed, false - unpressed
    bool GetEncoderButtonCurrentState(PortType port, EncButtonType button);

    // *************************************************************************
    // ***   Get encoder button state - CAN BE CALLED FROM ONE TASK   **********
    // *************************************************************************
    // Return button state: true - state changed, false - state remain
    bool GetEncoderButtonState(PortType port, EncButtonType button);

    // *************************************************************************
    // ***   Get encoder button state   ****************************************
    // *************************************************************************
    // Return button state: true - state changed, false - state remain
    bool GetEncoderButtonState(PortType port, EncButtonType button, bool& btn_state);

    // *************************************************************************
    // ***   Get joystick counts from last call   ******************************
    // *************************************************************************
    void GetJoystickState(PortType port, int32_t& x, int32_t& y);

    // *************************************************************************
    // ***   SetJoystickCalibrationConsts   ************************************
    // *************************************************************************
    // * Set calibration constants. Must be call for calibration joystick.
    void SetJoystickCalibrationConsts(PortType port, int32_t x_mid,
                                      int32_t x_kmin, int32_t x_kmax,
                                      int32_t y_mid, int32_t y_kmin,
                                      int32_t y_kmax);

    // *************************************************************************
    // ***   Get joystick button state   ***************************************
    // *************************************************************************
    // Return button state: true - pressed, false - unpressed
    bool GetJoystickButtonState(PortType port);

    // *************************************************************************
    // ***   Get joystick button state   ***************************************
    // *************************************************************************
    // Return button state: true - pressed, false - unpressed
    bool GetJoystickButtonState(PortType port, bool& btn_state);

  private:
    // How many cycles button must change state before state will be changed in
    // result returned by GetButtonState() function. For reduce debouncing
    const static uint32_t BUTTON_READ_DELAY = 4U;
    // Coefficient for calibration
    const static int32_t COEF = 100;

    // ADC max value - 12 bit
    const static int32_t ADC_MAX_VAL = 0xFFF;
    // Joystich threshold
    const static int32_t JOY_THRESHOLD = 1000;

    // Ticks variable
    uint32_t last_wake_ticks = 0U;

    // *************************************************************************
    // ***   Structure to describe button   ************************************
    // *************************************************************************
    typedef struct
    {
      bool btn_state;           // Button state returned by GetButtonState() function
      bool btn_state_tmp;       // Temporary button state for reduce debouncing
      uint8_t btn_state_cnt;    // Counter for reduce debouncing
      GPIO_TypeDef* button_port;// Button port
      uint16_t button_pin;      // Button pin
      GPIO_PinState pin_state;  // High/low on input treated as pressed
    } ButtonProfile;

    // *************************************************************************
    // ***   Structure to describe encoder   ***********************************
    // *************************************************************************
    typedef struct
    {
      // Encoder rotation
      int32_t enc_cnt;            // Encoder counter
      uint8_t enc_state;          // Current state of encder clock & data pins
      GPIO_TypeDef* enc_clk_port; // Encoder clock port
      uint16_t enc_clk_pin;       // Encoder clock pin
      GPIO_TypeDef* enc_data_port;// Encoder data port
      uint16_t enc_data_pin;      // Encoder data pin
    } EncoderProfile;

    // *************************************************************************
    // ***   Structure to describe joysticks   *********************************
    // *************************************************************************
    typedef struct
    {
      int32_t x_ch_val;     // Joystick X axis value
      uint32_t x_channel;   // Joystick X axis ADC channel
      GPIO_TypeDef* x_port; // Joystick X axis port
      uint16_t x_pin;       // Joystick X axis pin 
      int32_t bx;           // Joystick X offset
      int32_t kxmin;        // Joystick X coefficient
      int32_t kxmax;        // Joystick X coefficient
      bool x_inverted;      // Joystick X inverted flag
      int32_t y_ch_val;     // Joystick Y axis value
      uint32_t y_channel;   // Joystick Y axis ADC channel
      GPIO_TypeDef* y_port; // Joystick Y axis port
      uint16_t y_pin;       // Joystick Y axis pin
      int32_t by;           // Joystick Y offset
      int32_t kymin;        // Joystick Y coefficient
      int32_t kymax;        // Joystick Y coefficient
      bool y_inverted;      // Joystick Y inverted flag
    } JoystickProfile;

    // *************************************************************************
    // ***   Structure to describe encoders   **********************************
    // *************************************************************************
    typedef struct
    {
      EncoderProfile enc;
      ButtonProfile  btn[ENC_BTN_MAX];
    } DevEncoders;

    // *************************************************************************
    // ***   Structure to describe encoders   **********************************
    // *************************************************************************
    typedef struct
    {
      JoystickProfile joy;
      ButtonProfile   btn;
    } DevJoysticks;

    // *************************************************************************
    // ***   Structure to describe buttons   ***********************************
    // *************************************************************************
    typedef struct
    {
      ButtonProfile button[BTN_MAX];
    } DevButtons;

    // ***   Array describes types of connected devices  ***********************
    ExtDeviceType devices[EXT_MAX];

    // ***   Structures array for describe buttons inputs  *********************
    DevButtons buttons[EXT_MAX] =
    {
      // Left device
      {{{false, false, 0, EXT_L1_GPIO_Port, EXT_L1_Pin, GPIO_PIN_RESET},
        {false, false, 0, EXT_L2_GPIO_Port, EXT_L2_Pin, GPIO_PIN_RESET},
        {false, false, 0, EXT_L3_GPIO_Port, EXT_L3_Pin, GPIO_PIN_RESET},
        {false, false, 0, EXT_L4_GPIO_Port, EXT_L4_Pin, GPIO_PIN_RESET}}},
      // Right device
      {{{false, false, 0, EXT_R1_GPIO_Port, EXT_R1_Pin, GPIO_PIN_RESET},
        {false, false, 0, EXT_R2_GPIO_Port, EXT_R2_Pin, GPIO_PIN_RESET},
        {false, false, 0, EXT_R3_GPIO_Port, EXT_R3_Pin, GPIO_PIN_RESET},
        {false, false, 0, EXT_R4_GPIO_Port, EXT_R4_Pin, GPIO_PIN_RESET}}}
    };

    // ***   Structures array for describe encoders inputs  ********************
    DevEncoders encoders[EXT_MAX] =
    {
      // Left device
      {{0, 0, EXT_L1_GPIO_Port, EXT_L1_Pin, EXT_L2_GPIO_Port, EXT_L2_Pin}, // Encoder
       {{false, false, 0, EXT_L3_GPIO_Port, EXT_L3_Pin, GPIO_PIN_RESET},   // Button Enter
        {false, false, 0, EXT_L4_GPIO_Port, EXT_L4_Pin, GPIO_PIN_SET}}},   // Button Back
      // Right device
      {{0, 0, EXT_R1_GPIO_Port, EXT_R1_Pin, EXT_R2_GPIO_Port, EXT_R2_Pin}, // Encoder
       {{false, false, 0, EXT_R3_GPIO_Port, EXT_R3_Pin, GPIO_PIN_RESET},   // Button Enter
        {false, false, 0, EXT_R4_GPIO_Port, EXT_R4_Pin, GPIO_PIN_SET}}}    // Button Back
    };

    // ***   Structures array for describe encoders inputs  ********************
    DevJoysticks joysticks[EXT_MAX] =
    {
      // Left device
      {{0, ADC_CHANNEL_11, EXT_L2_GPIO_Port, EXT_L2_Pin, 0, COEF, COEF, false, // Joystick
        0, ADC_CHANNEL_10, EXT_L1_GPIO_Port, EXT_L1_Pin, 0, COEF, COEF, true},
       {false, false, 0, EXT_L3_GPIO_Port, EXT_L3_Pin, GPIO_PIN_RESET}},       // Button
      // Right device
      {{0, ADC_CHANNEL_13, EXT_R2_GPIO_Port, EXT_R2_Pin, 0, COEF, COEF, false, // Joystick
        0, ADC_CHANNEL_12, EXT_R1_GPIO_Port, EXT_R1_Pin, 0, COEF, COEF, true},
       {false, false, 0, EXT_R3_GPIO_Port, EXT_R3_Pin, GPIO_PIN_RESET}}        // Button
    };
 
    // Handle to timer used for process encoders input
    TIM_HandleTypeDef* htim = nullptr;
    // Handle to timer used for process encoders input
    ADC_HandleTypeDef* hadc = nullptr;

    // *************************************************************************
    // ***   Last value for reduce overhead in user task   *********************
    // *************************************************************************
    // Encoder values
    int32_t last_enc_value[EXT_MAX] = {0};
    // Encoder button states
    bool enc_btn_value[EXT_MAX][ENC_BTN_MAX] = {0};

    // *************************************************************************
    // ***   Process Button Input function   ***********************************
    // *************************************************************************
    void ProcessButtonInput(ButtonProfile& button);

    // *************************************************************************
    // ***   Process Encoder Input function   **********************************
    // *************************************************************************
    void ProcessEncoderInput(EncoderProfile& encoder);

    // *************************************************************************
    // ***   Process Joystick Input function   *********************************
    // *************************************************************************
    void ProcessJoystickInput(JoystickProfile& joysticks, PortType port);

    // *************************************************************************
    // ***   Emulate buttons using joystick function   *************************
    // *************************************************************************
    void EmulateButtonsByJoystick(PortType port);

    // *************************************************************************
    // ***   Emulate encoders using buttons function   *************************
    // *************************************************************************
    void EmulateEncodersByButtons(PortType port);

    // *************************************************************************
    // ***   Configure inputs devices types   **********************************
    // *************************************************************************
    ExtDeviceType DetectDeviceType(PortType port);

    // *************************************************************************
    // ***   Configure ADC   ***************************************************
    // *************************************************************************
    void ConfigADC(ExtDeviceType dev_left, ExtDeviceType dev_right);

    // *************************************************************************
    // ***   Configure inputs for read digital/analog data   *******************
    // *************************************************************************
    void ConfigInputIO(bool is_digital, PortType port);

    // *************************************************************************
    // ** Private constructor. Only GetInstance() allow to access this class. **
    // *************************************************************************
    InputDrv() : AppTask(INPUT_DRV_TASK_STACK_SIZE, INPUT_DRV_TASK_PRIORITY,
                         "InputDrv") {};
};

#endif

#endif
