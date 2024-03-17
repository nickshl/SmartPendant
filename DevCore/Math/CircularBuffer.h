//******************************************************************************
//  @file CircularBuffer.h
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

#ifndef CircularBuffer_h
#define CircularBuffer_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************

#include <limits>

// *****************************************************************************
// ***   Circular Buffer template class   **************************************
// *****************************************************************************
template <class T, int N> class CircularBuffer
{
  public:
    // *************************************************************************
    // ***   Public: CircularBuffer   ******************************************
    // *************************************************************************
    CircularBuffer()
    {
      // Clear array
      Clear();
    }

    // *************************************************************************
    // ***   Public: ~CircularBuffer   *****************************************
    // *************************************************************************
    ~CircularBuffer() {};

    // *************************************************************************
    // ***   Public: Add   *****************************************************
    // *************************************************************************
    void Add(T value)
    {
      // Store new value in array
      array[position] = value;
      // Increase position
      position++;
      // Check overflow
      if(position >= N)
      {
        // Clear position
        position = 0U;
        // Full array filled
        filled = true;
      }
    }

    // *************************************************************************
    // ***   Public: GetValueFromHead   ****************************************
    // *************************************************************************
    T GetValueFromHead(uint32_t n)
    {
      uint32_t idx = GetHeadIdx();
      // Check if n less than values
      if(n < GetItemsCnt())
      {
        // Add if necessary
        if(n > idx) idx += N;
        idx -= n; 
      }
      // Return result
      return array[idx]; 
    }

    // *************************************************************************
    // ***   Public: GetValueFromTail   ****************************************`
    // *************************************************************************
    T GetValueFromTail(uint32_t n)
    {
      uint32_t idx = GetTailIdx();
      // Check if n less than values
      if(n < GetItemsCnt())
      {
        idx += n;
        // Decrease if necessary
        if(idx >= N) idx -= N;
      }
      // Return result
      return array[idx]; 
    }

    // *************************************************************************
    // ***   Public: IsFilled   ************************************************
    // *************************************************************************
    bool IsFilled(void)
    {
      return filled;
    }

    // *************************************************************************
    // ***   Public: GetBufferPointer   ****************************************
    // *************************************************************************
    T* GetBufferPointer(void)
    {
      // Return result
      return array;
    }

    // *************************************************************************
    // ***   Public: GetHeadIdx   **********************************************
    // *************************************************************************
    uint32_t GetHeadIdx(void)
    {
      // Local variable for store index
      uint32_t head_idx = 0U;
      // Find head index
      if(position != 0U)
      {
        head_idx = position - 1U;
      }
      else
      {
        if(filled == true)
        {
          head_idx = N - 1U;
        }
      }
      // Return result
      return head_idx;
    }

    // *************************************************************************
    // ***   Public: GetTailIdx   **********************************************
    // *************************************************************************
    uint32_t GetTailIdx(void)
    {
      // Local variable for store index
      uint32_t tail_idx = 0U;
      // Find tail index
      if(filled == false)
      {
        tail_idx = 0U;
      }
      else
      {
        tail_idx = position;
      }
      // Return result
      return tail_idx;
    }

    // *************************************************************************
    // ***   Public: GetItemsCnt   *********************************************
    // *************************************************************************
    uint32_t GetItemsCnt(void)
    {
      // Position by default
      uint32_t n = position;
      // If array full filled
      if(filled == true)
      {
        // Calculate average using all elements
        n = N;
      }
      // Return result
      return n;
    }

    // *************************************************************************
    // ***   Public: GetMinValue   *********************************************
    // *************************************************************************
    T GetMinValue(void)
    {
      // Items count
      uint32_t n = GetItemsCnt();

      // Clear max value
      T min_val = std::numeric_limits<T>::max();
      // Calculate sum from array
      for(uint32_t i = 0U; i < n; i++)
      {
        if(array[i] < min_val)
        {
          min_val = array[i];
        }
      }

      // Return result
      return min_val;
    }

    // *************************************************************************
    // ***   Public: GetMaxValue   *********************************************
    // *************************************************************************
    T GetMaxValue(void)
    {
      // Items count
      uint32_t n = GetItemsCnt();

      // Clear max value
      T max_val = std::numeric_limits<T>::min();
      // Calculate sum from array
      for(uint32_t i = 0U; i < n; i++)
      {
        if(array[i] > max_val)
        {
          max_val = array[i];
        }
      }

      // Return result
      return max_val;
    }

    // *************************************************************************
    // ***   Public: GetSum   **************************************************
    // *************************************************************************
    T GetSum(void)
    {
      // Clear sum
      T sum = 0;
      // Store count
      uint32_t n = GetItemsCnt();
      // Calculate sum from array
      for(uint32_t i = 0U; i < n; i++)
      {
        sum += array[i];
      }
      // Return result
      return sum;
    }

    // *************************************************************************
    // ***   Public: GetSumByCnt   *********************************************
    // *************************************************************************
    T GetSumByCnt(uint32_t& cnt)
    {
      // Get head and tail indexes
      int32_t head_idx = (int32_t)CircularBuffer<T,N>::GetHeadIdx();
      int32_t tail_idx = (int32_t)CircularBuffer<T,N>::GetTailIdx();

      // Head can be equal to position only if buffer is empty
      if(head_idx == position)
      {
        cnt = 0U;
      }

      // Sum
      T result = 0;
      // Counter
      uint32_t i = 0U;
      // Cycle while we have items and sum less than desired
      while(i < cnt)
      {
        // Increase counter
        i++;
        // Add value
        result += CircularBuffer<T,N>::array[head_idx];
        // If this is last element - head bite tail
        if(head_idx == tail_idx)
        {
          break;
        }
        // Decrease head index
        head_idx--;
        // Check underflow
        if(head_idx < 0)
        {
          head_idx = N - 1;
        }
      }

      // Overwrite input data with actual sum
      cnt = i;

      // Return result
      return result;
    }

    // *************************************************************************
    // ***   Public: GetCntBySum   *********************************************
    // *************************************************************************
    uint32_t GetCntBySum(T& sum)
    {
      // Get head and tail indexes
      int32_t head_idx = (int32_t)CircularBuffer<T,N>::GetHeadIdx();
      int32_t tail_idx = (int32_t)CircularBuffer<T,N>::GetTailIdx();

      // Head can be equal to position only if buffer is empty
      if(head_idx == position)
      {
        sum = 0;
      }

      T result = 0;
      uint32_t cnt = 0U;
      // Cycle while we have items and sum less than desired
      while(result < sum)
      {
        // Increase counter
        cnt++;
        // Add value
        result += CircularBuffer<T,N>::array[head_idx];
        // If this is last element - head bite tail
        if(head_idx == tail_idx)
        {
          break;
        }
        // Decrease head index
        head_idx--;
        // Check underflow
        if(head_idx < 0)
        {
          head_idx = N - 1;
        }
      }

      // Overwrite input data with actual sum
      sum = result;

      // Return result
      return cnt;
    }

    // *************************************************************************
    // ***   Public: GetAverage   **********************************************
    // *************************************************************************
    T GetAverage(void)
    {
      // Find average
      T avg = GetSum() / static_cast<T>(GetItemsCnt());
      // Return result
      return avg;
    }

    // *************************************************************************
    // ***   Public: Clear   ***************************************************
    // *************************************************************************
    void Clear(void)
    {
      // Clear position
      position = 0U;
      // Clear full fill flag
      filled = false;
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
    int32_t position = 0U;

    // Flag for track fill full array
    bool filled = false;

  private:
};

#endif
