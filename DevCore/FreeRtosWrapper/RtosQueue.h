//******************************************************************************
//  @file RtosMutex.h
//  @author Nicolai Shlapunov
//
//  @details DevCore: FreeRTOS Queue Wrapper Class, header
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

#ifndef RtosQueue_h
#define RtosQueue_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "Rtos.h"
#include "queue.h"

// ******************************************************************************
// ***   RtosQueue   ************************************************************
// ******************************************************************************
class RtosQueue
{
  public:
    // Maximum queue name length
    static const uint16_t MAX_QUEUE_NAME_LEN = 24U;

    // *************************************************************************
    // ***   RtosQueue   *******************************************************
    // *************************************************************************
    RtosQueue(uint32_t q_len, uint32_t itm_size, const char* queue_name = nullptr);

    // *************************************************************************
    // ***   ~RtosQueue   ******************************************************
    // *************************************************************************
    ~RtosQueue();

    // *************************************************************************
    // ***   SetName   *********************************************************
    // *************************************************************************
    void SetName(const char* name, const char* add_name = nullptr);

    // *************************************************************************
    // ***   Create   **********************************************************
    // *************************************************************************
    Result Create();

    // *************************************************************************
    // ***   Reset   ***********************************************************
    // *************************************************************************
    Result Reset();

    // *************************************************************************
    // ***   IsEmpty   *********************************************************
    // *************************************************************************
    bool IsEmpty() const;

    // *************************************************************************
    // ***   IsFull   **********************************************************
    // *************************************************************************
    bool IsFull() const;

    // *************************************************************************
    // ***   GetMessagesWaiting   **********************************************
    // *************************************************************************
    Result GetMessagesWaiting(uint32_t& msg_cnt) const;

    // *************************************************************************
    // ***   SendToBack   ******************************************************
    // *************************************************************************
    Result SendToBack(const void* item, uint32_t timeout_ms = 0U);

    // *************************************************************************
    // ***   SendToFront   *****************************************************
    // *************************************************************************
    Result SendToFront(const void* item, uint32_t timeout_ms = 0U);

    // *************************************************************************
    // ***   Receive   *********************************************************
    // *************************************************************************
    Result Receive(void* item, uint32_t timeout_ms);

    // *************************************************************************
    // ***   Peek   ************************************************************
    // *************************************************************************
    Result Peek(void* item, uint32_t timeout_ms) const;

    // *************************************************************************
    // ***   GetQueueLen   *****************************************************
    // *************************************************************************
    inline uint16_t GetQueueLen(void) const {return queue_len;}

    // *************************************************************************
    // ***   GetItemSize   *****************************************************
    // *************************************************************************
    inline uint16_t GetItemSize(void) const {return item_size;}

  private:
    // Queue handle
    QueueHandle_t queue;

    // Number of items in the queue
    uint16_t queue_len = 0u;

    // Size of item
    uint16_t item_size = 0u;

    // Queue name
    char queue_name[MAX_QUEUE_NAME_LEN];

    // Prevent copying and assigning
    RtosQueue();
    RtosQueue(const RtosQueue&);
    RtosQueue& operator=(const RtosQueue&);
};


#endif // FREE_RTOS_QUEUE_H
