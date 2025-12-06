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

#include "Crc32.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
NVM& NVM::GetInstance()
{
  static NVM nvm;
  return nvm;
}

// *****************************************************************************
// ***   Init function   *******************************************************
// *****************************************************************************
Result NVM::Init(Eeprom24& in_eep)
{
  Result result = Result::RESULT_OK;

  // Save EEP pointer
  eep = &in_eep;

  // Return result
  return result;
}

// *****************************************************************************
// ***   ReadData function   ***************************************************
// *****************************************************************************
Result NVM::ReadData()
{
  Result result = Result::RESULT_OK;

  // Read all NVM content
  result = eep->Read(0u, (uint8_t*)&data, sizeof(data));

  // Save CRC of read data to determinate later if settings was changed
  eep_crc = data.crc;
  // Calculate CRC32(without stored CRC)
  uint32_t crc = Crc32((uint8_t*)&data, sizeof(data) - sizeof(data.crc));

  // If we can't read data from chip or if CRC doesn't match - fill it with defaults values
  if(result.IsBad() || (crc != data.crc))
  {
    Nvm_t defaults;
    data = defaults;
    // Set bad CRC result only if we read data correctly
    if(result.IsGood()) result = Result::ERR_BAD_CRC;
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   WriteData function   **************************************************
// *****************************************************************************
Result NVM::WriteData(void)
{
  Result result = Result::RESULT_OK;

  // Calculate CRC32(without stored CRC)
  data.crc = Crc32((uint8_t*)&data, sizeof(data) - sizeof(data.crc));

  // If data actually changed
  if(data.crc != eep_crc)
  {
    // Write all into EEPROM
    result = eep->Write(0u, (uint8_t*)&data, sizeof(data));
    // If write is successful
    if(result.IsGood())
    {
      // Update CRC to match actual data in EEPROM
      eep_crc = data.crc;
    }
  }

  // Return result
  return result;
}
