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

  if(result.IsGood())
  {
    // Calculate CRC32(without stored CRC)
    uint32_t crc = Crc32((uint8_t*)&data, sizeof(data) - sizeof(data.crc));

    // Check CRC
    if(crc != data.crc)
    {
      // If it doesn't match - fill it with defaults values
      Nvm_t defaults;
      data = defaults;
    }
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

  // Write all into EEPROM
  result = eep->Write(0u, (uint8_t*)&data, sizeof(data));

  // Return result
  return result;
}
