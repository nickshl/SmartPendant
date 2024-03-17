//******************************************************************************
//  @file ISpi.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: SPI driver interface, header
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

#ifndef ISpi_h
#define ISpi_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"

// *****************************************************************************
// ***   SPI Driver Interface   ************************************************
// *****************************************************************************
class ISpi
{
  public:
    // *************************************************************************
    // ***   Public: SPI Mode enum   *******************************************
    // *************************************************************************
    enum Mode
    {
      MODE_0 = 0,
      MODE_1,
      MODE_2,
      MODE_3
    };

    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    explicit ISpi() {};

    // *************************************************************************
    // ***   Public: Destructor   **********************************************
    // *************************************************************************
    virtual ~ISpi() {};

    // *************************************************************************
    // ***   Public: Init   ****************************************************
    // *************************************************************************
    virtual Result Init() = 0;

    // *************************************************************************
    // ***   Public: DeInit   **************************************************
    // *************************************************************************
    virtual Result DeInit() {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Transfer   ************************************************
    // *************************************************************************
    virtual Result Transfer(uint8_t* tx_buf_ptr, uint8_t* rx_buf_ptr, uint32_t size) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Write   ***************************************************
    // *************************************************************************
    virtual Result Write(uint8_t* tx_buf_ptr, uint32_t tx_size) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Read   ****************************************************
    // *************************************************************************
    virtual Result Read(uint8_t* rx_buf_ptr, uint32_t rx_size) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: TransferAsync   *******************************************
    // *************************************************************************
    virtual Result TransferAsync(uint8_t* tx_buf_ptr, uint8_t* rx_buf_ptr, uint32_t size) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: WriteAsync   **********************************************
    // *************************************************************************
    virtual Result WriteAsync(uint8_t* tx_buf_ptr, uint32_t tx_size) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: ReadAsync   ***********************************************
    // *************************************************************************
    virtual Result ReadAsync(uint8_t* rx_buf_ptr, uint32_t rx_size) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Check SPI transfer status   *******************************
    // *************************************************************************
    virtual bool IsTransferComplete(void) {return true;}

    // *************************************************************************
    // ***   Public: Abort   ***************************************************
    // *************************************************************************
    virtual Result Abort(void) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: SetSpeed   ************************************************
    // *************************************************************************
    virtual Result SetSpeed(uint32_t clock_rate) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: GetSpeed   ************************************************
    // *************************************************************************
    virtual Result GetSpeed(uint32_t& clock_rate) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: SetMode   *************************************************
    // *************************************************************************
    virtual Result SetMode(Mode mode) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: GetMode   *************************************************
    // *************************************************************************
    virtual Result GetMode(Mode& mode) {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: SetTxTimeout   ********************************************
    // *************************************************************************
    virtual void SetTxTimeout(uint16_t timeout_ms) {spi_tx_timeout_ms = timeout_ms;}

    // *************************************************************************
    // ***   Public: SetRxTimeout   ********************************************
    // *************************************************************************
    virtual void SetRxTimeout(uint16_t timeout_ms) {spi_rx_timeout_ms = timeout_ms;}

  protected:
    // Timeout for SPI TX operation
    uint16_t spi_tx_timeout_ms = 100U;

    // Timeout for SPI RX operation
    uint16_t spi_rx_timeout_ms = 100U;

  private:
    // *************************************************************************
    // ***   Private: Constructors and assign operator - prevent copying   *****
    // *************************************************************************
    ISpi(const ISpi&);
};

#endif
