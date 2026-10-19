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
#include "DevCore.h"

#include "Version.h"

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
    enum Parameters : uint16_t
    {
      // General: 1 - 31
      TX_CONTROL = 1,
      BAUD_RATE = 2,
      TRANSPORT = 3,
      FRAME_ATTEMPTS = 4,
      ACK_MIN_MS = 5,
      AUTO_MPG_ON_START = 6,
      SAVE_SCRIPT_RESULT = 7,
      SCREEN_INVERT = 8,
      // MPG: 32 - 63
      MPG_MATCH_SPEED_LIMITS = 32,
      MPG_METRIC_FEED_1 = 33,
      MPG_METRIC_FEED_2 = 34,
      MPG_METRIC_FEED_3 = 35,
      MPG_METRIC_FEED_4 = 36,
      MPG_IMPERIAL_FEED_1 = 37,
      MPG_IMPERIAL_FEED_2 = 38,
      MPG_IMPERIAL_FEED_3 = 39,
      MPG_IMPERIAL_FEED_4 = 40,
      MPG_ROTARY_FEED_1 = 41,
      MPG_ROTARY_FEED_2 = 42,
      MPG_ROTARY_FEED_3 = 43,
      MPG_ROTARY_FEED_4 = 44,
      // Probe: 64 - 95
      PROBE_SEARCH_FEED = 64,
      PROBE_LOCK_FEED = 65,
      PROBE_POS_DEVIATION = 66,
      PROBE_BALL_TIP = 67,
    };

    // Number of banks. Used for wear leveling.
    static constexpr uint32_t BANKS_CNT = 2u;

    // *************************************************************************
    // ***   Public: Get Instance   ********************************************
    // *************************************************************************
    static NVM& GetInstance();

    // *************************************************************************
    // ***   Public: Init function   *******************************************
    // *************************************************************************
    Result Init(Eeprom24& in_eep);

    // *************************************************************************
    // ***   Public: GetReadResult function   **********************************
    // *************************************************************************
    Result GetReadResult();

    // *************************************************************************
    // ***   Public: GetValue function   ***************************************
    // *************************************************************************
    int32_t GetValue(Parameters idx);

    // *************************************************************************
    // ***   Public: SetValue function   ***************************************
    // *************************************************************************
    Result SetValue(Parameters param, int32_t value);

    // *************************************************************************
    // ***   Public: GetCtrlTx function   **************************************
    // *************************************************************************
    uint8_t GetCtrlTx() {return GetValue(TX_CONTROL);}

    // *************************************************************************
    // ***   Public: SetCtrlTx function   **************************************
    // *************************************************************************
    void SetCtrlTx(uint8_t tx_ctrl) {SetValue(TX_CONTROL, tx_ctrl);}

  private:

    // Pointer to EEPROM object
    Eeprom24* eep = nullptr;
    // Sizes of EEP
    uint16_t eep_bank_size = 0u;
    uint16_t eep_page_size = 0u;

    // Record in EEPROM, one per page
    typedef struct
    {
      uint16_t id;    // Parameter ID: rejects data read from the wrong page
      uint16_t seq;   // Incremented on every change: newer of two banks
      int32_t value;  // Parameter value
      uint32_t crc;   // CRC32 of all fields above
    } EepRecord_t;

    // Parameter
    typedef struct
    {
      const Parameters id;     // Parameter ID
      int32_t value = 0;       // Current value, set to default by constructor
      uint16_t seq = 0u;       // Current sequence number
      uint16_t page = 0u;       // Page where sits parameter stored in EEP
      uint8_t bank = 0u;       // Current bank
      uint8_t valid = 0u;      // Flags
    } Param_t;

    // Array with all NVM parameters - object is singleton, so static is fine
    static Param_t data[];
    // Size of data - which is show how many parameters we have
    static const uint32_t PARAMS_CNT;

    // Result of the last ReadData() call
    Result read_result = Result::RESULT_OK;

    // *************************************************************************
    // ***   Private: ReadData function   **************************************
    // *************************************************************************
    Result ReadData(void);

    // *************************************************************************
    // ***   Private: WriteData function   *************************************
    // *************************************************************************
    Result WriteData(Parameters param);

    // *************************************************************************
    // ***   Private: GetIdxByParam function   *********************************
    // *************************************************************************
    uint32_t GetIdxByParam(uint16_t param);

    // *************************************************************************
    // ***   Private: IsPageUsed function   ************************************
    // *************************************************************************
    bool IsPageUsed(uint32_t page);

    // *************************************************************************
    // ***   Private: GetFreePage function   ***********************************
    // *************************************************************************
    uint16_t GetFreePage();

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    NVM() {};
};

#endif
