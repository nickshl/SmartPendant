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

  // Read all NVM content
  result = eep->Read(0u, (uint8_t*)&data, sizeof(data));

  // TODO: CRC check and reset to defaults

  // Return result
  return result;
}

// *****************************************************************************
// ***   SetCtrlTx function   **************************************************
// *****************************************************************************
Result NVM::SetCtrlTx(uint8_t tx_ctrl)
{
  Result result = Result::RESULT_OK;

  // Save data
  data.tx_ctrl = tx_ctrl;

  // Write data into EEPROM
  result = eep->Write((uint8_t*)&data.tx_ctrl - (uint8_t*)&data, (uint8_t*)&data.tx_ctrl, sizeof(data.tx_ctrl));

  // Return result
  return result;
}

// *****************************************************************************
// ***   SetMode function   **************************************************
// *****************************************************************************
Result NVM::SetMode(uint8_t mode)
{
  Result result = Result::RESULT_OK;

  // Save data
  data.mode = mode;

  // Write data into EEPROM
  result = eep->Write((uint8_t*)&data.mode - (uint8_t*)&data, (uint8_t*)&data.mode, sizeof(data.mode));

  // Return result
  return result;
}

// *****************************************************************************
// ***   SetDisplayInvert function   *******************************************
// *****************************************************************************
Result NVM::SetDisplayInvert(bool invert_display)
{
  Result result = Result::RESULT_OK;

  // Save data
  data.invert_display = invert_display;

  // Write data into EEPROM
  result = eep->Write((uint8_t*)&data.invert_display - (uint8_t*)&data, (uint8_t*)&data.invert_display, sizeof(data.invert_display));

  // Return result
  return result;
}

// *****************************************************************************
// ***   SetUnits function   ***************************************************
// *****************************************************************************
Result NVM::SetUnits(bool units)
{
  Result result = Result::RESULT_OK;

  // Save data
  data.is_metric = units;

  // Write data into EEPROM
  result = eep->Write((uint8_t*)&data.is_metric - (uint8_t*)&data, (uint8_t*)&data.is_metric, sizeof(data.is_metric));

  // Return result
  return result;
}
