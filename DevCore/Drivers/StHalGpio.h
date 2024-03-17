//******************************************************************************
//  @file StHalGpio.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: STM32 HAL GPIO driver, header
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

#ifndef StHalGpio_h
#define StHalGpio_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "IGpio.h"

// *****************************************************************************
// ***   This driver can be compiled only if GPIO configured in CubeMX   *******
// *****************************************************************************
#ifndef HAL_GPIO_MODULE_ENABLED
  typedef uint32_t GPIO_TypeDef;
#endif

// *****************************************************************************
// ***   STM32 HAL I2C Driver Class   ******************************************
// *****************************************************************************
class StHalGpio : public IGpio
{
  public:
    // *************************************************************************
    // ***   Public: Constructor   *********************************************
    // *************************************************************************
    StHalGpio(GPIO_TypeDef* port, uint32_t pin, Type type_parm, Polarity polarity_parm = LOW) :
      IGpio(type_parm, polarity_parm), gpio_port(port), pin_mask_bit(pin) {};

    // *************************************************************************
    // ***   Public: Read   ****************************************************
    // *************************************************************************
    virtual Polarity Read();

    // *************************************************************************
    // ***   Public: Write   ***************************************************
    // *************************************************************************
    virtual void Write(Polarity polarity);

    // *************************************************************************
    // ***   Public: GetPort   *************************************************
    // *************************************************************************
    inline GPIO_TypeDef* GetPort()
    {
      return gpio_port;
    }

    // *************************************************************************
    // ***   Public: GetPin   **************************************************
    // *************************************************************************
    inline uint32_t GetPin()
    {
      return pin_mask_bit;
    }

  private:
    // Pointer to the GPIO register set as provided by CubeMX
    GPIO_TypeDef* gpio_port;

    // Port pin bit mask as defined by CubeMX
    uint32_t pin_mask_bit;
};

#endif
