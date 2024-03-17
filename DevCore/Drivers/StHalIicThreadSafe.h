//******************************************************************************
//  @file StHalIicThreadSafe.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: STM32 HAL I2C thread safe driver, header
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

#ifndef StHalIicThreadSafe_h
#define StHalIicThreadSafe_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "IIic.h"

// *****************************************************************************
// ***   This driver can be compiled only if UART configured in CubeMX   *******
// *****************************************************************************
#ifndef HAL_I2C_MODULE_ENABLED
  typedef uint32_t I2C_HandleTypeDef; // Dummy I2C handle for header compilation
#endif

// *****************************************************************************
// ***   STM32 HAL I2C Driver Class   ******************************************
// *****************************************************************************
class StHalIicThreadSafe : public IIic
{
  public:
    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    explicit StHalIicThreadSafe(I2C_HandleTypeDef& hi2c_ref) : hi2c(hi2c_ref) {};

    // *************************************************************************
    // ***   Public: Destructor   **********************************************
    // *************************************************************************
    ~StHalIicThreadSafe() {};

    // *************************************************************************
    // ***   Public: Init   ****************************************************
    // *************************************************************************
    virtual Result Init() {return Result::ERR_NOT_IMPLEMENTED;}

    // *************************************************************************
    // ***   Public: Enable   **************************************************
    // *************************************************************************
    virtual Result Enable();

    // *************************************************************************
    // ***   Public: Disable   *************************************************
    // *************************************************************************
    virtual Result Disable();

    // *************************************************************************
    // ***   Public: Reset   ***************************************************
    // *************************************************************************
    virtual Result Reset();

    // *************************************************************************
    // ***   Public: IsDeviceReady   *******************************************
    // *************************************************************************
    virtual Result IsDeviceReady(uint16_t addr, uint8_t retries = 1U);

    // *************************************************************************
    // ***   Public: Transfer   ************************************************
    // *************************************************************************
    virtual Result Transfer(uint16_t addr, uint8_t* tx_buf_ptr, uint32_t tx_size,
                            uint8_t* rx_buf_ptr, uint32_t rx_size);

    // *************************************************************************
    // ***   Public: Write   ***************************************************
    // *************************************************************************
    virtual Result Write(uint16_t addr, uint8_t* tx_buf_ptr, uint32_t tx_size);

    // *************************************************************************
    // ***   Public: Read   ****************************************************
    // *************************************************************************
    virtual Result Read(uint16_t addr, uint8_t* rx_buf_ptr, uint32_t rx_size);

    // *************************************************************************
    // ***   Public: WriteAsync   **********************************************
    // *************************************************************************
    virtual Result WriteAsync(uint16_t addr, uint8_t* tx_buf_ptr, uint32_t tx_size);

    // *************************************************************************
    // ***   Public: ReadAsync   ***********************************************
    // *************************************************************************
    virtual Result ReadAsync(uint16_t addr, uint8_t* rx_buf_ptr, uint32_t rx_size);

    // *************************************************************************
    // ***   Public: IsBusy   **************************************************
    // *************************************************************************
    virtual bool IsBusy(void);

  private:
    // Reference to the I2C handle
    I2C_HandleTypeDef& hi2c;

    // Mutex for synchronize when reads touch coordinates
    RtosMutex mutex;

    // *************************************************************************
    // ***   Private: ConvertResult   ******************************************
    // *************************************************************************
    Result ConvertResult(HAL_StatusTypeDef hal_result);

    // *************************************************************************
    // ***   Private: Constructors and assign operator - prevent copying   *****
    // *************************************************************************
    StHalIicThreadSafe();
    StHalIicThreadSafe(const StHalIicThreadSafe&);
    StHalIicThreadSafe& operator=(const StHalIicThreadSafe);
};

#endif
