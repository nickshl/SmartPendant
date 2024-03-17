//******************************************************************************
//  @file IIic.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: I2C driver interface, header
//
//  @section LICENSE
//
//   Software License Agreement (Modified BSD License)
//
//   Copyright (c) 2018, Devtronic & Nicolai Shlapunov
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

#ifndef IIic_h
#define IIic_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"

// *****************************************************************************
// ***   I2C Driver Interface   ************************************************
// *****************************************************************************
class IIic
{
  public:
    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    explicit IIic() {};

    // *************************************************************************
    // ***   Public: Destructor   **********************************************
    // *************************************************************************
    virtual ~IIic() {};

    // *************************************************************************
    // ***   Public: Init   ****************************************************
    // *************************************************************************
    virtual Result Init() = 0;

    // *************************************************************************
    // ***   Public: DeInit   **************************************************
    // *************************************************************************
    virtual Result DeInit() {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Enable   **************************************************
    // *************************************************************************
    virtual Result Enable() {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Disable   *************************************************
    // *************************************************************************
    virtual Result Disable() {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Reset   ***************************************************
    // *************************************************************************
    virtual Result Reset() {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: IsDeviceReady   *******************************************
    // *************************************************************************
    virtual Result IsDeviceReady(uint16_t addr, uint8_t retries = 1U) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Transfer   ************************************************
    // *************************************************************************
    virtual Result Transfer(uint16_t addr, uint8_t* tx_buf_ptr, uint32_t tx_size, uint8_t* rx_buf_ptr, uint32_t rx_size) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Write   ***************************************************
    // *************************************************************************
    virtual Result Write(uint16_t addr, uint8_t* tx_buf_ptr, uint32_t tx_size) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Read   ****************************************************
    // *************************************************************************
    virtual Result Read(uint16_t addr, uint8_t* rx_buf_ptr, uint32_t rx_size) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: WriteAsync   **********************************************
    // *************************************************************************
    virtual Result WriteAsync(uint16_t addr, uint8_t* tx_buf_ptr, uint32_t tx_size) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: ReadAsync   ***********************************************
    // *************************************************************************
    virtual Result ReadAsync(uint16_t addr, uint8_t* rx_buf_ptr, uint32_t rx_size) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: IsBusy   **************************************************
    // *************************************************************************
    virtual bool IsBusy(void) {return false;}

    // *************************************************************************
    // ***   Public: SetTxTimeout   ********************************************
    // *************************************************************************
    virtual void SetTxTimeout(uint16_t timeout_ms) {i2c_tx_timeout_ms = timeout_ms;}

    // *************************************************************************
    // ***   Public: SetRxTimeout   ********************************************
    // *************************************************************************
    virtual void SetRxTimeout(uint16_t timeout_ms) {i2c_rx_timeout_ms = timeout_ms;}

  protected:
    // Timeout for I2C TX operation
    uint16_t i2c_tx_timeout_ms = 5U;

    // Timeout for I2C RX operation
    uint16_t i2c_rx_timeout_ms = 5U;

  private:
    // *************************************************************************
    // ***   Private: Constructors and assign operator - prevent copying   *****
    // *************************************************************************
    IIic(const IIic&);
};

#endif
