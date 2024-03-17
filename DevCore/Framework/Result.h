//******************************************************************************
//  @file Result.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: Result codes, header
//
//  @section LICENSE
//
//   Software License Agreement (Modified BSD License)
//
//   Copyright (c) 2016, Devtronic & Nicolai Shlapunov
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

#ifndef Result_h
#define Result_h

// *****************************************************************************
// ***   Result   **************************************************************
// *****************************************************************************
class Result
{
  public:
    // *************************************************************************
    // ***   Enum with all possible result codes   *****************************
    // *************************************************************************
    enum ResultCode
    {
      // ***   No error   ******************************************************
      RESULT_OK = 0,

      // ***   Generic   *******************************************************
      ERR_NULL_PTR,
      ERR_BAD_PARAMETER,
      ERR_INVALID_ITEM,
      ERR_NOT_IMPLEMENTED,
      ERR_BAD_CRC,
      ERR_BUSY,
      ERR_OVERFLOW,
      ERR_UNHANDLED_REQUEST,
      ERR_INVALID_COMMAND,
      ERR_INVALID_SIZE,
      ERR_INVALID_DATA,
      ERR_CANNOT_EXECUTE,

      // ***   RTOS errors   ***************************************************
      ERR_TASK_CREATE,
      ERR_QUEUE_CREATE,
      ERR_QUEUE_GENERAL,
      ERR_QUEUE_EMPTY,
      ERR_QUEUE_READ,
      ERR_QUEUE_WRITE,
      ERR_QUEUE_RESET,
      ERR_TIMER_CREATE,
      ERR_TIMER_START,
      ERR_TIMER_UPDATE,
      ERR_TIMER_STOP,
      ERR_MUTEX_CREATE,
      ERR_MUTEX_LOCK,
      ERR_MUTEX_RELEASE,
      ERR_SEMAPHORE_CREATE,
      ERR_SEMAPHORE_TAKE,
      ERR_SEMAPHORE_GIVE,

      // ***   UART errors   ***************************************************
      ERR_UART_GENERAL,
      ERR_UART_TRANSMIT,
      ERR_UART_RECEIVE,
      ERR_UART_EMPTY,
      ERR_UART_BUSY,
      ERR_UART_TIMEOUT,
      ERR_UART_UNKNOWN,

      // ***   I2C errors   ****************************************************
      ERR_I2C_GENERAL,
      ERR_I2C_BUSY,
      ERR_I2C_TIMEOUT,
      ERR_I2C_UNKNOWN,

      // ***   SPI errors   ****************************************************
      ERR_SPI_GENERAL,
      ERR_SPI_BUSY,
      ERR_SPI_TIMEOUT,
      ERR_SPI_UNKNOWN,

      // ***   Elements count   ************************************************
      RESULTS_CNT
    };

    // *************************************************************************
    // ***   Result   **********************************************************
    // *************************************************************************
    Result() {};

    // *************************************************************************
    // ***   Result   **********************************************************
    // *************************************************************************
    Result(ResultCode res)
    {
      result = res;
    }

    // *************************************************************************
    // ***   IsGood   **********************************************************
    // *************************************************************************
    bool IsGood() const
    {
      return result == RESULT_OK;
    }

    // *************************************************************************
    // ***   IsBad   ***********************************************************
    // *************************************************************************
    bool IsBad() const
    {
      return result != RESULT_OK;
    }

    // *************************************************************************
    // ***   operator ResultCode   *********************************************
    // *************************************************************************
    operator ResultCode() const
    {
      return result;
    }

    // *************************************************************************
    // ***   operator=   *******************************************************
    // *************************************************************************
    Result& operator=(ResultCode res)
    {
      result = res;
      return *this;
    }

    // *************************************************************************
    // ***   operator=   *******************************************************
    // *************************************************************************
    Result& operator=(const Result& r_arg)
    {
      result = r_arg.result;
      return *this;
    }

    // *************************************************************************
    // ***   operator|=   ******************************************************
    // *************************************************************************
    Result& operator|=(ResultCode res)
    {
      if(result == RESULT_OK)
      {
        result = res;
      }
      return *this;
    }

    // *************************************************************************
    // ***   operator|=   ******************************************************
    // *************************************************************************
    Result& operator|=(const Result& r_arg)
    {
      if(result == RESULT_OK)
      {
        result = r_arg.result;
      }
      return *this;
    }

    // *************************************************************************
    // ***   operator==   ******************************************************
    // *************************************************************************
    bool operator==(ResultCode res) const
    {
       return(result == res);
    }

    // *************************************************************************
    // ***   operator==   ******************************************************
    // *************************************************************************
    bool operator==(const Result& r_arg) const
    {
      return result == r_arg.result;
    }

    // *************************************************************************
    // ***   operator!=   ******************************************************
    // *************************************************************************
    bool operator!=(ResultCode res) const
    {
      return(result != res);
    }

    // *************************************************************************
    // ***   operator!=   ******************************************************
    // *************************************************************************
    bool operator!=(const Result& r_arg) const
    {
      return(result != r_arg.result);
    }

  private:
    // Result code
    ResultCode result = RESULT_OK;
};

#endif
