//******************************************************************************
//  @file NVM.h
//  @author Nicolai Shlapunov
//
//  @details NVM: NVM Class, header
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

#ifndef NVM_h
#define NVM_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"

#include "Eeprom24.h"

// *****************************************************************************
// ***   Local const variables   ***********************************************
// *****************************************************************************

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************

// *****************************************************************************
// ***   NVM Class   ***********************************************************
// *****************************************************************************
class NVM
{
  public:
    // Enum with menu items
    enum Parameters
    {
      // General
      TX_CONTROL,
      SCREEN_INVERT,
      // MPG
      MPG_METRIC_FEED_1,
      MPG_METRIC_FEED_2,
      MPG_METRIC_FEED_3,
      MPG_METRIC_FEED_4,
      MPG_IMPERIAL_FEED_1,
      MPG_IMPERIAL_FEED_2,
      MPG_IMPERIAL_FEED_3,
      MPG_IMPERIAL_FEED_4,
      MPG_ROTARY_FEED_1,
      MPG_ROTARY_FEED_2,
      MPG_ROTARY_FEED_3,
      MPG_ROTARY_FEED_4,
      // Probe
      PROBE_SEARCH_FEED,
      PROBE_LOCK_FEED,
      PROBE_BALL_TIP,
      // Total
      MAX_VALUES
    };

    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static NVM& GetInstance();

    // *************************************************************************
    // ***   Init function   ***************************************************
    // *************************************************************************
    Result Init(Eeprom24& in_eep);

    // *************************************************************************
    // ***   ReadData function   ***********************************************
    // *************************************************************************
    Result ReadData(void);

    // *************************************************************************
    // ***   WriteData function   **********************************************
    // *************************************************************************
    Result WriteData(void);

    // *************************************************************************
    // ***   GetValue function   ***********************************************
    // *************************************************************************
    int32_t GetValue(Parameters idx) {return data.value[idx];}

    // *************************************************************************
    // ***   SetValue function   ***********************************************
    // *************************************************************************
    void SetValue(Parameters idx, int32_t value) {data.value[idx] = value;}

    // *************************************************************************
    // ***   GetCtrlTx function   **********************************************
    // *************************************************************************
    uint8_t GetCtrlTx() {return data.value[TX_CONTROL];}

    // *************************************************************************
    // ***   SetCtrlTx function   **********************************************
    // *************************************************************************
    void SetCtrlTx(uint8_t tx_ctrl) {data.value[TX_CONTROL] = tx_ctrl;}

    // *************************************************************************
    // ***   GetDisplayInvert function   ***************************************
    // *************************************************************************
    bool GetDisplayInvert() {return data.value[SCREEN_INVERT];}

    // *************************************************************************
    // ***   SetDisplayInvert function   ***************************************
    // *************************************************************************
    void SetDisplayInvert(bool invert_display) {data.value[SCREEN_INVERT] = invert_display;}

  private:

    // Pointer to EEPROM object
    Eeprom24* eep = nullptr;

    // Struct with all settings we want to store in EEPROM
    typedef struct
    {
      int32_t value[MAX_VALUES] =
      {
        // General
        2,    // TX_CONTROL
        0,    // SCREEN_INVERT
        // MPG
        1,    // MPG_METRIC_FEED_1: 0.001 mm
        5,    // MPG_METRIC_FEED_2: 0.005 mm
        10,   // MPG_METRIC_FEED_3: 0.010 mm
        100,  // MPG_METRIC_FEED_4: 0.100 mm
        1,    // MPG_IMPERIAL_FEED_1: 0.0001 inch
        2,    // MPG_IMPERIAL_FEED_2: 0.0002 inch
        5,    // MPG_IMPERIAL_FEED_3: 0.0005 inch
        50,   // MPG_IMPERIAL_FEED_4: 0.0050 inch
        180,  // MPG_ROTARY_FEED_1: 0.180 deg - 20 handwheel revolution for full turn
        360,  // MPG_ROTARY_FEED_2: 0.360 deg - 10 handwheel revolution for full turn
        720,  // MPG_ROTARY_FEED_3: 0.720 deg -  5 handwheel revolution for full turn
        3600, // MPG_ROTARY_FEED_4: 3.600 deg -  1 handwheel revolution for full turn
        // Probe
        200,  // PROBE_SEARCH_FEED
        50,   // PROBE_LOCK_FEED
        2000  // PROBE_BALL_TIP
      };
      // CRC
      uint32_t crc = 0u;
    } Nvm_t;
    // Data
    Nvm_t data;

    // CRC of data in EEPROM. Used to track if settings was changed.
    uint32_t eep_crc = 0u;

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    NVM() {};
};

#endif
