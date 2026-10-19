//******************************************************************************
//  @file NVM.cpp
//  @author Nicolai Shlapunov
//
//  @details NVM: NVM Class, implementation
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
#include "NVM.h"

// Array with all NVM parameters
NVM::Param_t NVM::data[] =
{
  // General
  {TX_CONTROL, 2, 0u, 0u, 0u, 0u},
  {BAUD_RATE, 115200, 0u, 0u, 0u, 0u},
  {TRANSPORT, 0, 0u, 0, 0u, 0u},
  {FRAME_ATTEMPTS, 4, 0u, 0u, 0u, 0u},
  {ACK_MIN_MS, 0, 0u, 0u, 0u, 0u},
  {AUTO_MPG_ON_START, 0, 0u, 0u, 0u, 0u},
  {SAVE_SCRIPT_RESULT, 0, 0u, 0u, 0u, 0u},
  {SCREEN_INVERT, 0, 0u, 0u, 0u, 0u},
  // MPG
  {MPG_MATCH_SPEED_LIMITS, 0, 0u, 0u, 0u, 0u},
  {MPG_METRIC_FEED_1, 1, 0u, 0u, 0u, 0u},
  {MPG_METRIC_FEED_2, 5, 0u, 0u, 0u, 0u},
  {MPG_METRIC_FEED_3, 10, 0u, 0u, 0u, 0u},
  {MPG_METRIC_FEED_4, 100, 0u, 0u, 0u, 0u},
  {MPG_IMPERIAL_FEED_1, 1, 0u, 0u, 0u, 0u},
  {MPG_IMPERIAL_FEED_2, 2, 0u, 0u, 0u, 0u},
  {MPG_IMPERIAL_FEED_3, 5, 0u, 0u, 0u, 0u},
  {MPG_IMPERIAL_FEED_4, 50, 0u, 0u, 0u, 0u},
  {MPG_ROTARY_FEED_1, 180, 0u, 0u, 0u, 0u},
  {MPG_ROTARY_FEED_2, 360, 0u, 0u, 0u, 0u},
  {MPG_ROTARY_FEED_3, 720, 0u, 0u, 0u, 0u},
  {MPG_ROTARY_FEED_4, 3600, 0u, 0u, 0u, 0u},
  // Probe
  {PROBE_SEARCH_FEED, 200, 0u, 0u, 0u, 0u},
  {PROBE_LOCK_FEED, 50, 0u, 0u, 0u, 0u},
  {PROBE_POS_DEVIATION, 10, 0u, 0u, 0u, 0u},
  {PROBE_BALL_TIP, 2000, 0u, 0u, 0u, 0u}
};

// Number of parameters
const uint32_t NVM::PARAMS_CNT = NumberOf(NVM::data);

// *****************************************************************************
// ***   Public: Get Instance   ************************************************
// *****************************************************************************
NVM& NVM::GetInstance()
{
  static NVM nvm;
  return nvm;
}

// *****************************************************************************
// ***   Public: GetReadResult function   **************************************
// *****************************************************************************
Result NVM::GetReadResult()
{
  // Assign pages and save data only if all reads successful - prevents
  // loosing data in case of I2C glitch. It done there and not in ReadData()
  // because we want write data only after scheduler is started. This
  // function called from Application::Setup(), which satisfy this condition.
  if(read_result.IsGood())
  {
    // Check that all parameters read
    for(uint32_t idx = 0u; idx < PARAMS_CNT; idx++)
    {
      // If valid flag isn't set for any of parameter
      if(data[idx].valid == 0u)
      {
        // Assign NVM page for new parameter
        data[idx].page = GetFreePage();
        // Save parameter to NVM
        SetValue(data[idx].id, data[idx].value);
        // Save it again to update both copies
        SetValue(data[idx].id, data[idx].value);
        // Set valid flag for GetFreePage() to work
        data[idx].valid = true;
        // It mean we have no data in EEP and those parameters defaulted
        read_result = Result::ERR_INVALID_DATA;
      }
    }
  }

  return read_result;
}

// *****************************************************************************
// ***   Public: Init function   ***********************************************
// *****************************************************************************
Result NVM::Init(Eeprom24& in_eep)
{
  Result result = Result::RESULT_OK;

  // Save EEP pointer
  eep = &in_eep;

  // Get EEP sizes
  eep_bank_size = eep->GetSize() / BANKS_CNT;
  eep_page_size = eep->GetPageSize();

  // Read data and populate struct
  ReadData();

  // Return result
  return result;
}

// *****************************************************************************
// ***   Public: GetValue function   *******************************************
// *****************************************************************************
int32_t NVM::GetValue(Parameters param)
{
  // Value is zero by default(but should never happen)
  int32_t value = 0;

  // Find value
  for(uint32_t i = 0u; i < PARAMS_CNT; i++)
  {
    if(data[i].id == param)
    {
      value = data[i].value;
      break;
    }
  }

  // Return value
  return value;
}

// *****************************************************************************
// ***   Public: SetValue function   *******************************************
// *****************************************************************************
Result NVM::SetValue(Parameters param, int32_t value)
{
  Result result = Result::RESULT_OK;

  // Record is correct - store it in structure
  uint16_t idx = GetIdxByParam(param);

  // If parameter was found
  if(idx != PARAMS_CNT)
  {
    // Record variable used to read data
    EepRecord_t record = {0};

    // Fill record to write
    record.id = param;
    record.value = value;
    record.seq = data[idx].seq + 1u; // Increase sequence number
    // Fill CRC
    record.crc = Crc32((uint8_t*)&record, sizeof(record) - sizeof(record.crc));

    // Find bank we need to write
    uint32_t bank = (data[idx].bank + 1u) % BANKS_CNT;

    // Write record into EEPROM
    result = eep->Write(bank * eep_bank_size + data[idx].page * eep_page_size, (uint8_t*)&record, sizeof(record));

    // Update bank and sequence number only if write is successful
    if(result.IsGood())
    {
      data[idx].value = value;
      data[idx].bank = bank;
      data[idx].seq = record.seq;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: ReadData function   ******************************************
// *****************************************************************************
Result NVM::ReadData()
{
  Result result = Result::RESULT_OK;

  // Record variable used to read data
  EepRecord_t record = {0};

  // Read all data and populate structure
  for(uint32_t bank = 0u; bank < BANKS_CNT; bank++)
  {
    // Variable to count parameters to stop after all read
    uint16_t parameters_read_cnt = 0u;
    // Check bank
    for(uint32_t page = 0u; (page < (eep_bank_size / eep_page_size)) && (parameters_read_cnt < PARAMS_CNT); page++)
    {
      // Read NVM record
      result = eep->Read(bank * eep_bank_size + page * eep_page_size, (uint8_t*)&record, sizeof(record));
      // Check result
      if(result.IsGood())
      {
        // Calculate CRC32(without stored CRC)
        uint32_t crc = Crc32((uint8_t*)&record, sizeof(record) - sizeof(record.crc));
        // Check CRC
        if(crc == record.crc)
        {
          // Record is correct - store it in structure
          uint16_t idx = GetIdxByParam(record.id);
          // If parameter was found
          if(idx != PARAMS_CNT)
          {
            // If parameter valid and found - count it
            parameters_read_cnt++;
            // If data in copy is newer
            if(!data[idx].valid || (int16_t)(record.seq - data[idx].seq) > 0)
            {
              // Save data
              data[idx].value = record.value;
              data[idx].seq = record.seq;
              data[idx].bank = bank;
              data[idx].page = page;
              data[idx].valid = 1u;
            }
          }
          else
          {
            // Unknown parameter. Old firmware loaded on device with newer one
            // that had more parameters?
          }
        }
      }
      else
      {
        // EEP read error
        break;
      }
    }
    // EEP read error
    if(result.IsBad()) break;
  }

  // Save read result
  read_result = result;

  // Return result
  return result;
}

// *****************************************************************************
// ***   Private: WriteData function   *****************************************
// *****************************************************************************
Result NVM::WriteData(Parameters param)
{
  Result result = Result::RESULT_OK;

  // Record is correct - store it in structure
  uint16_t idx = GetIdxByParam(param);

  // If parameter was found
  if(idx != PARAMS_CNT)
  {
    // Record variable used to read data
    EepRecord_t record = {0};

    // Fill record to write
    record.id = data[idx].id;
    record.value = data[idx].value;
    record.seq = data[idx].seq + 1u; // Seq should be increased
    // Fill CRC
    record.crc = Crc32((uint8_t*)&record, sizeof(record) - sizeof(record.crc));

    // Find bank we need to write
    uint32_t bank = (data[idx].bank + 1u) % BANKS_CNT;

    // Write record into EEPROM
    result = eep->Write(bank * eep_bank_size + data[idx].page * eep_page_size, (uint8_t*)&record, sizeof(record));

    // Update bank and sequence number only if write is successful
    if(result.IsGood())
    {
      data[idx].bank = bank;
      data[idx].seq = record.seq;
    }
  }

  // Return result
  return result;
}


// *****************************************************************************
// ***   Private: GetIdxByParam function   *************************************
// *****************************************************************************
uint32_t NVM::GetIdxByParam(uint16_t param)
{
  uint32_t idx = 0u;

  // Find index
  for(idx = 0u; idx < PARAMS_CNT; idx++)
  {
    if(data[idx].id == param)
    {
      break;
    }
  }

  return idx;
}

// *****************************************************************************
// ***   Private: IsPageUsed function   ****************************************
// *****************************************************************************
bool NVM::IsPageUsed(uint32_t page)
{
  bool result = false;

  for(uint32_t i = 0u; i < PARAMS_CNT; i++)
  {
    if(data[i].valid && (data[i].page == page))
    {
      result = true;
      break;
    }
  }

  return result;
}

// *****************************************************************************
// ***   Private: GetFreePage function   ***************************************
// *****************************************************************************
uint16_t NVM::GetFreePage()
{
  uint16_t page = 0u;

  while((page < eep_bank_size / eep_page_size) && IsPageUsed(page)) page++;

  return page;
}
