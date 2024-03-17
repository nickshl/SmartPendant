//******************************************************************************
//  @file UpdateAreaProcessor.h
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

#ifndef UpdateAreaProcessor_h
#define UpdateAreaProcessor_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DisplayDrv.h"

// *****************************************************************************
// ***   Struct to describe update area   **************************************
// *****************************************************************************
typedef struct UpdateArea
{
  // Update area start position
  uint16_t start_x = 0u;
  uint16_t end_x = 0u;
  uint16_t start_y = 0u;
  uint16_t end_y = 0u;
} UpdateArea_t;

#if defined(MULTIPLE_UPDATE_AREAS)

// *****************************************************************************
// ***   Circular Buffer template class   **************************************
// *****************************************************************************
template <int N> class UpdateAreaProcessor
{
  public:
    // *************************************************************************
    // ***   Public: FIFO   ****************************************************
    // *************************************************************************
    UpdateAreaProcessor()
    {
      // Clear array
      Clear();
    }

    // *************************************************************************
    // ***   Public: ~FIFO   ***************************************************
    // *************************************************************************
    ~UpdateAreaProcessor() {};

    // *************************************************************************
    // ***   Public: Push   ****************************************************
    // *************************************************************************
    bool Push(UpdateArea_t& value)
    {
      // False by default
      bool result = false;

      // Try to find filled spot
      for(uint32_t idx = 0u; idx < N; idx++)
      {
        if(!IsEmpty(idx))
        {
          // Check intersection
          bool width_is_positive  = MIN(value.end_x, array[idx].end_x) > MAX(value.start_x, array[idx].start_x);
          bool height_is_positive = MIN(value.end_y, array[idx].end_y) > MAX(value.start_y, array[idx].start_y);
          // Merge areas if intersects
          if(width_is_positive && height_is_positive)
          {
            // Add new area to existing one
            if(value.start_x < array[idx].start_x) array[idx].start_x = value.start_x;
            if(value.start_y < array[idx].start_y) array[idx].start_y = value.start_y;
            if(value.end_x > array[idx].end_x) array[idx].end_x = value.end_x;
            if(value.end_y > array[idx].end_y) array[idx].end_y = value.end_y;
            // Set result
            result = true;
            // Already merged, break the cycle
            break;
          }
        }
      }
      // If isn't intersect with any other areas
      if(result == false)
      {
        // Find empty spot
        uint32_t idx = GetFirstEmptySpot();
        // Check if there an empty spot in the list
        if(idx < N)
        {
          // Store new value in array
          array[idx] = value;
          // Set result
          result = true;
        }
        else // otherwise
        {
          // Add new area to the first existing one
          if(value.start_x < array[0].start_x) array[0].start_x = value.start_x;
          if(value.start_y < array[0].start_y) array[0].start_y = value.start_y;
          if(value.end_x > array[0].end_x) array[0].end_x = value.end_x;
          if(value.end_y > array[0].end_y) array[0].end_y = value.end_y;
          // Set result to try merge areas that can be in between
          result = true;
        }
      }
      // If area was merged, try to re-merge other areas
      if(result == true)
      {
        // Merge all areas
        while(ReMerge());
        // Move other records to beginning
        Compact();
      }
      // Return result
      return result;
    }

    // *************************************************************************
    // ***   Public: Pop   *****************************************************
    // *************************************************************************
    bool Pop(UpdateArea_t& val)
    {
      // False by default
      bool result = false;
      // Find filled spot
      uint32_t idx = GetFirstFilledSpot();
      // If we have one
      if(idx < N)
      {
        // Store value
        val = array[idx];
        // Clear the spot
        array[idx] = {0};
        // Move other records to beginning
        Compact();
        // Set result
        result = true;
      }
      // Return result
      return result;
    }

    // *************************************************************************
    // ***   Public: IsEmpty   *************************************************
    // *************************************************************************
    bool IsEmpty(uint32_t idx)
    {
      return ( (array[idx].start_x == 0) && (array[idx].start_y == 0) &&
               (array[idx].end_x == 0) && (array[idx].end_y == 0) );
    }

    // *************************************************************************
    // ***   Public: IsEmpty   *************************************************
    // *************************************************************************
    bool IsEmpty(void)
    {
      // Empty by default
      bool flag = true;
      // Try to find filled spot
      for(uint32_t i = 0u; i < N; i++)
      {
        if( (array[i].start_x != 0) || (array[i].start_y != 0) ||
            (array[i].end_x != 0) || (array[i].end_y != 0) )
        {
          flag = false;
          break;
        }
      }
      return flag;
    }

    // *************************************************************************
    // ***   Public: IsFull   **************************************************
    // *************************************************************************
    bool IsFull(void)
    {
      // Full by default
      bool flag = true;
      // try to find empty spot
      for(uint32_t i = 0u; i < N; i++)
      {
        if( (array[i].start_x == 0) && (array[i].start_y == 0) &&
            (array[i].end_x == 0) && (array[i].end_y == 0) )
        {
          flag = false;
          break;
        }
      }
      return flag;
    }

    // *************************************************************************
    // ***   Public: GetItemsCnt   *********************************************
    // *************************************************************************
    uint32_t GetItemsCnt(void)
    {
      uint32_t n = 0;
      // Count filled spots
      for(uint32_t i = 0u; i < N; i++)
      {
        if( (array[i].start_x != 0) || (array[i].start_y != 0) ||
            (array[i].end_x != 0) || (array[i].end_y != 0) )
        {
          n++;
        }
      }
      // Return result
      return n;
    }

    // *************************************************************************
    // ***   Public: GetFirstEmptySpot   ***************************************
    // *************************************************************************
    uint32_t GetFirstEmptySpot(void)
    {
      uint32_t idx = 0;
      // Count filled spots
      for(; idx < N; idx++)
      {
        if( (array[idx].start_x == 0) && (array[idx].start_y == 0) &&
            (array[idx].end_x == 0) && (array[idx].end_y == 0) )
        {
          break;
        }
      }
      // Return result
      return idx;
    }

    // *************************************************************************
    // ***   Public: GetFirstFilledSpot   **************************************
    // *************************************************************************
    uint32_t GetFirstFilledSpot(void)
    {
      uint32_t idx = 0;
      // Count filled spots
      for(; idx < N; idx++)
      {
        if( (array[idx].start_x != 0) || (array[idx].start_y != 0) ||
            (array[idx].end_x != 0) || (array[idx].end_y != 0) )
        {
          break;
        }
      }
      // Return result
      return idx;
    }

    // *************************************************************************
    // ***   Public: ReMerge   *************************************************
    // *************************************************************************
    bool ReMerge(void)
    {
      // No merge happened
      bool result = false;
      // Count filled spots
      for(uint32_t idx = 0; idx < N; idx++)
      {
        // If current record isn't empty
        if(!IsEmpty(idx))
        {
          // Check the rest
          for(uint32_t n = idx + 1u; n < N; n++)
          {
            // If we found isn't empty area - try to merge it
            if(!IsEmpty(n))
            {
              // Check intersection
              bool width_is_positive  = MIN(array[n].end_x, array[idx].end_x) > MAX(array[n].start_x, array[idx].start_x);
              bool height_is_positive = MIN(array[n].end_y, array[idx].end_y) > MAX(array[n].start_y, array[idx].start_y);
              // Merge areas if intersects
              if(width_is_positive && height_is_positive)
              {
                // Add new area to existing one
                if(array[n].start_x < array[idx].start_x) array[idx].start_x = array[n].start_x;
                if(array[n].start_y < array[idx].start_y) array[idx].start_y = array[n].start_y;
                if(array[n].end_x > array[idx].end_x) array[idx].end_x = array[n].end_x;
                if(array[n].end_y > array[idx].end_y) array[idx].end_y = array[n].end_y;
                // Clear merged record
                array[n] = {0};
                // Set result
                result = true;
                // Already merged, break the cycle
                break;
              }
            }
          }
        }
      }
      // Return result
      return result;
    }

    // *************************************************************************
    // ***   Public: Compact   *************************************************
    // *************************************************************************
    void Compact(void)
    {
      // Count all records
      for(uint32_t idx = 0; idx < N; idx++)
      {
        // If current record is empty
        if(IsEmpty(idx))
        {
          // Check the rest
          for(uint32_t n = idx + 1u; n < N; n++)
          {
            // If we found isn't empty area - copy and clear it
            if(!IsEmpty(n))
            {
              // Copy record closer to beginning of array
              array[idx] = array[n];
              // Clear merged record
              array[n] = {0};
              // Break cycle
              break;
            }
          }
        }
      }
    }

    // *************************************************************************
    // ***   Public: Clear   ***************************************************
    // *************************************************************************
    void Clear(void)
    {
      // Clear array
      for(uint32_t i = 0U; i < N; i++)
      {
        array[i] = {0};
      }
    }

  protected:
    // Array for calculating average
    UpdateArea_t array[N];

  private:
};

#endif

#endif
