//******************************************************************************
//  @file Hysteresis.h
//  @author Nicolai Shlapunov
//
//  @details Hysteresis template class, header
//
//  @section LICENSE
//
//   Software License Agreement (Modified BSD License)
//
//   Copyright (c) 2023, Devtronic & Nicolai Shlapunov
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

#ifndef Hysteresis_h
#define Hysteresis_h

// *****************************************************************************
// ***   Circular Buffer template class   **************************************
// *****************************************************************************
template <class T> class Hysteresis
{
  public:
    // *************************************************************************
    // ***   Public: Hysteresis   **********************************************
    // *************************************************************************
    Hysteresis(T min_in, T max_in) : min(min_in), max(max_in) {};

    // *************************************************************************
    // ***   Public: ~Hysteresis   *********************************************
    // *************************************************************************
    ~Hysteresis() {};

    // *************************************************************************
    // ***   Public: Check   ***************************************************
    // *************************************************************************
    bool Process(T val)
    {
      state = ((val > max) && !state) || ((val > min) && state);
      return state;
    } 

  protected:
    // Min value for hysteresis
    T min;
    // Max value for hysteresis
    T max;
    // Flag for track hysteresis state
    bool state = false;

  private:
};

#endif
