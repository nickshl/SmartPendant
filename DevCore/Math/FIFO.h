//******************************************************************************
//  @file FIFO.h
//  @author Nicolai Shlapunov
//
//  @details Circular Buffer template class, header
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

#ifndef FIFO_h
#define FIFO_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************

#include <limits>

// *****************************************************************************
// ***   Circular Buffer template class   **************************************
// *****************************************************************************
template <class T, int N> class FIFO
{
  public:
    // *************************************************************************
    // ***   Public: FIFO   ****************************************************
    // *************************************************************************
    FIFO()
    {
      // Clear array
      Clear();
    }

    // *************************************************************************
    // ***   Public: ~FIFO   ***************************************************
    // *************************************************************************
    ~FIFO() {};

    // *************************************************************************
    // ***   Public: Push   ****************************************************
    // *************************************************************************
    bool Push(T value)
    {
      // False by default
      bool result = false;
      // Check if we can add value
      if(GetItemsCnt() < N - 1)
      {
        // Store new value in array
        array[head_idx] = value;
        // Increase position
        head_idx++;
        // Check overflow
        if(head_idx >= N)
        {
          // Clear position
          head_idx = 0U;
        }
        // Set result
        result = true;
      }
      // Return result
      return result;
    }

    // *************************************************************************
    // ***   Public: PushUnique   **********************************************
    // *************************************************************************
    bool PushUnique(T value)
    {
      // False by default
      bool result = false;
      // Check if value is already there
      int32_t n = tail_idx;
      while(n != head_idx)
      {
        // If value found
        if(value == array[n])
        {
          array[n] |= value;
          result = true;
          break;
        }
        // Increase position
        n++;
        // Check overflow
        if(n >= N)
        {
          // Clear position
          n = 0u;
        }
      }
      // Check if we can add value
      if((result != true) && (GetItemsCnt() < N - 1))
      {
        // Store new value in array
        array[head_idx] = value;
        // Increase position
        head_idx++;
        // Check overflow
        if(head_idx >= N)
        {
          // Clear position
          head_idx = 0U;
        }
        // Set result
        result = true;
      }
      // Return result
      return result;
    }

    // *************************************************************************
    // ***   Public: Pop   *****************************************************
    // *************************************************************************
    bool Pop(T& val)
    {
      // False by default
      bool result = false;
      // If queue isn't empty
      if(tail_idx != head_idx)
      {
        // Store value
        val = array[tail_idx];
        // Increase position
        tail_idx++;
        // Check overflow
        if(tail_idx >= N)
        {
          // Clear position
          tail_idx = 0U;
        }
        // Set result
        result = true;
      }
      // Return result
      return result;
    }

    // *************************************************************************
    // ***   Public: IsEmpty   *************************************************
    // *************************************************************************
    bool IsEmpty(void)
    {
      return (head_idx == tail_idx);
    }

    // *************************************************************************
    // ***   Public: IsFull   **************************************************
    // *************************************************************************
    bool IsFull(void)
    {
      return (head_idx == tail_idx);
    }

    // *************************************************************************
    // ***   Public: GetItemsCnt   *********************************************
    // *************************************************************************
    uint32_t GetItemsCnt(void)
    {
      uint32_t n = 0;
      // If head less than tail - add buffer length
      if(head_idx < tail_idx)
      {
        n = N + head_idx - tail_idx;
      }
      else
      {
        n = head_idx - tail_idx;
      }
      // Return result
      return n;
    }

    // *************************************************************************
    // ***   Public: Clear   ***************************************************
    // *************************************************************************
    void Clear(void)
    {
      // Clear head
      head_idx = 0U;
      // Clear tail
      tail_idx = 0U;
      // Clear array
      for(uint32_t i = 0U; i < N; i++)
      {
        array[i] = {0};
      }
    }

  protected:
    // Array for calculating average
    T array[N];

    // Position in array for next add operation
    int32_t head_idx = 0U;
    int32_t tail_idx = 0U;

  private:
};

#endif
