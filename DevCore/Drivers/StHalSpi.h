//******************************************************************************
//  @file StHalSpi.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: STM32 HAL SPI driver, header
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

#ifndef StmHalSpi_h
#define StmHalSpi_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "ISpi.h"

// *****************************************************************************
// ***   This driver can be compiled only if UART configured in CubeMX   *******
// *****************************************************************************
#ifndef HAL_SPI_MODULE_ENABLED
  typedef uint32_t SPI_HandleTypeDef; // Dummy SPI handle for header compilation
#endif

// *****************************************************************************
// ***   STM32 HAL ISPI  Driver Class   ****************************************
// *****************************************************************************
class StHalSpi : public ISpi
{
  public:
    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    explicit StHalSpi(SPI_HandleTypeDef& hspi_ref) : hspi(hspi_ref) {};

    // *************************************************************************
    // ***   Public: Destructor   **********************************************
    // *************************************************************************
    ~StHalSpi() {};

    // *************************************************************************
    // ***   Public: Init   ****************************************************
    // *************************************************************************
    virtual Result Init() {return Result::RESULT_OK;}

    // *************************************************************************
    // ***   Public: DeInit   **************************************************
    // *************************************************************************
    virtual Result DeInit() {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Transfer   ************************************************
    // *************************************************************************
    virtual Result Transfer(uint8_t* tx_buf_ptr, uint8_t* rx_buf_ptr, uint32_t size);

    // *************************************************************************
    // ***   Public: Write   ***************************************************
    // *************************************************************************
    virtual Result Write(uint8_t* tx_buf_ptr, uint32_t tx_size);

    // *************************************************************************
    // ***   Public: Read   ****************************************************
    // *************************************************************************
    virtual Result Read(uint8_t* rx_buf_ptr, uint32_t rx_size);

    // *************************************************************************
    // ***   Public: TransferAsync   *******************************************
    // *************************************************************************
    virtual Result TransferAsync(uint8_t* tx_buf_ptr, uint8_t* rx_buf_ptr, uint32_t size);

    // *************************************************************************
    // ***   Public: WriteAsync   **********************************************
    // *************************************************************************
    virtual Result WriteAsync(uint8_t* tx_buf_ptr, uint32_t tx_size);

    // *************************************************************************
    // ***   Public: ReadAsync   ***********************************************
    // *************************************************************************
    virtual Result ReadAsync(uint8_t* rx_buf_ptr, uint32_t rx_size);

    // *************************************************************************
    // ***   Public: Check SPI transfer status   *******************************
    // *************************************************************************
    virtual bool IsTransferComplete(void);

    // *************************************************************************
    // ***   Public: Abort   ***************************************************
    // *************************************************************************
    virtual Result Abort(void);

    // *************************************************************************
    // ***   Public: SetSpeed   ************************************************
    // *************************************************************************
    virtual Result SetSpeed(uint32_t clock_rate);

    // *************************************************************************
    // ***   Public: GetSpeed   ************************************************
    // *************************************************************************
    virtual Result GetSpeed(uint32_t& clock_rate);

    // *************************************************************************
    // ***   Public: SetMode   *************************************************
    // *************************************************************************
    virtual Result SetMode(ISpi::Mode mode);

    // *************************************************************************
    // ***   Public: GetMode   *************************************************
    // *************************************************************************
    virtual Result GetMode(ISpi::Mode& mode);

  private:
    // Reference to the SPI handle
    SPI_HandleTypeDef& hspi;

    // *************************************************************************
    // ***   Private: GetToDataSizeBytes   *************************************
    // *************************************************************************
    uint32_t GetToDataSizeBytes();

    // *************************************************************************
    // ***   Private: ConvertResult   ******************************************
    // *************************************************************************
    Result ConvertResult(HAL_StatusTypeDef hal_result);

    // *************************************************************************
    // ***   Private: Constructors and assign operator - prevent copying   *****
    // *************************************************************************
    StHalSpi();
    StHalSpi(const StHalSpi&);
    StHalSpi& operator=(const StHalSpi);
};

#endif
