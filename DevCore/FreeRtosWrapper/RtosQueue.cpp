//******************************************************************************
//  @file RtosQueue.cpp
//  @author Nicolai Shlapunov
//
//  @details DevCore: FreeRTOS Rtos Wrapper Class, implementation
//
//  @copyright Copyright (c) 2018, Devtronic & Nicolai Shlapunov
//             All rights reserved.
//
//  @section SUPPORT
//
//   Devtronic invests time and resources providing this open source code,
//   please support Devtronic and open-source hardware/software by
//   donations and/or purchasing products from Devtronic.
//
//******************************************************************************

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "RtosQueue.h"
#include "Rtos.h"

// *****************************************************************************
// ***   RtosQueue   ***********************************************************
// *****************************************************************************
RtosQueue::RtosQueue(uint32_t q_len, uint32_t itm_size, const char* queue_name)
   : queue(nullptr), queue_len(q_len), item_size(itm_size)
{
  // Set name
  SetName(queue_name);
}

// *****************************************************************************
// ***   ~RtosQueue   **********************************************************
// *****************************************************************************
RtosQueue::~RtosQueue()
{
  // Check queue handle
  if(queue != nullptr)
  {
    vQueueDelete(queue);
  }
}

// *****************************************************************************
// ***   ~RtosQueue   **********************************************************
// *****************************************************************************
void RtosQueue::SetName(const char* name, const char* add_name)
{
  uint32_t i = 0U;
  // If name pointer isn't null
  if(name != nullptr)
  {
    // Copy queue name
    for(; (i < MAX_QUEUE_NAME_LEN - 1U) && (name[i] != '\0'); i++)
    {
      queue_name[i] = name[i];
    }
  }
  // If additional name pointer isn't null
  if(add_name != nullptr)
  {
    // Copy additional queue name
    for(uint32_t j = 0U; (i < MAX_QUEUE_NAME_LEN - 1U) && (add_name[j] != '\0'); i++, j++)
    {
      queue_name[i] = add_name[j];
    }
  }
  // Set null-terminator for string
  queue_name[i] = '\0';
}

// *****************************************************************************
// ***   Create   **************************************************************
// *****************************************************************************
Result RtosQueue::Create()
{
  Result result = Result::ERR_QUEUE_CREATE;

  // Check queue handle
  if(queue == nullptr)
  {
    // Create queue
    queue = xQueueCreate(queue_len, item_size);

    // Check result
    if(queue != nullptr)
    {
      // If name present - add to registry
      if(queue_name[0] != '\0')
      {
        vQueueAddToRegistry(queue, queue_name);
      }
      // Set result
      result = Result::RESULT_OK;
    }
  }

  // Return result
  return result;
}

// *****************************************************************************
// ***   Reset   ***************************************************************
// *****************************************************************************
Result RtosQueue::Reset()
{
  Result result = Result::ERR_QUEUE_RESET;

  // Check queue handle
  if(queue != nullptr)
  {
    // Reset queue
    if(xQueueReset(queue) == pdPASS)
    {
      // Set result
      result = Result::RESULT_OK;
    }
  }

  // Return result
  return result;
}


// *****************************************************************************
// ***   IsEmpty   *************************************************************
// *****************************************************************************
bool RtosQueue::IsEmpty() const
{
  bool is_empty = false;

  // Check queue handle
  if(queue != nullptr)
  {
    // Check handler mode
    if(Rtos::IsInHandlerMode())
    {
      // Check is queue empty
      is_empty = (xQueueIsQueueEmptyFromISR(queue) != pdFALSE);
    }
    else
    {
      // Check is queue empty
      is_empty = (uxQueueMessagesWaiting(queue) == 0U);
    }
  }

  return is_empty;
}

// *****************************************************************************
// ***   IsFull   **************************************************************
// *****************************************************************************
bool RtosQueue::IsFull() const
{
  bool is_full = false;

  // Check queue handle
  if(queue != nullptr)
  {
    // Check handler mode
    if(Rtos::IsInHandlerMode())
    {
      // Check is queue full
      is_full = (xQueueIsQueueFullFromISR(queue) != pdFALSE);
    }
    else
    {
      // Check is queue full
      is_full = (uxQueueMessagesWaiting(queue) >= queue_len);
    }
  }

  return is_full;
}

// *****************************************************************************
// ***   GetMessagesWaiting   **************************************************
// *****************************************************************************
Result RtosQueue::GetMessagesWaiting(uint32_t& msg_cnt) const
{
  Result result = Result::ERR_QUEUE_GENERAL;

  // Check queue handle
  if(queue != nullptr)
  {
    // Check handler mode
    if(Rtos::IsInHandlerMode())
    {
      msg_cnt = uxQueueMessagesWaitingFromISR(queue);
    }
    else
    {
      msg_cnt = uxQueueMessagesWaiting(queue);
    }
    // Set result
    result = Result::RESULT_OK;
  }

  return result;
}

// *****************************************************************************
// ***   SendToBack   **********************************************************
// *****************************************************************************
Result RtosQueue::SendToBack(const void* item, uint32_t timeout_ms)
{
  Result result = Result::ERR_QUEUE_WRITE;

  // Check queue handle and item pointer
  if((queue != nullptr) && (item != nullptr))
  {
    // Result of operation
    portBASE_TYPE res = pdFALSE;

    // Check handler mode
    if(Rtos::IsInHandlerMode())
    {
      BaseType_t task_woken;
      // Send item to back of queue from ISR
      res = xQueueSendToBackFromISR(queue, item, &task_woken);
      // Switch context if needed
      portEND_SWITCHING_ISR((res == pdPASS) ? task_woken : pdFALSE);
    }
    else
    {
      // Send item to back of queue
      res = xQueueSendToBack(queue, item, RtosTick::MsToTicks(timeout_ms));
    }
    // Check result
    if(res == pdPASS)
    {
      result = Result::RESULT_OK;
    }
  }

  return result;
}

// *****************************************************************************
// ***   SendToFront   *********************************************************
// *****************************************************************************
Result RtosQueue::SendToFront(const void* item, uint32_t timeout_ms)
{
  Result result = Result::ERR_QUEUE_WRITE;

  // Check queue handle and item pointer
  if((queue != nullptr) && (item != nullptr))
  {
    // Result of operation
    portBASE_TYPE res = pdFALSE;

    // Check handler mode
    if(Rtos::IsInHandlerMode())
    {
      BaseType_t task_woken;
      // Send item to front of queue from ISR
      res = xQueueSendToFrontFromISR(queue, item, &task_woken);
      // Switch context if needed
      portEND_SWITCHING_ISR((res == pdPASS) ? task_woken : pdFALSE);
    }
    else
    {
      // Send item to front of queue
      res = xQueueSendToFront(queue, item, RtosTick::MsToTicks(timeout_ms));
    }
    // Check result
    if(res == pdPASS)
    {
      result = Result::RESULT_OK;
    }
  }

  return result;
}

// *****************************************************************************
// ***   Receive   *************************************************************
// *****************************************************************************
Result RtosQueue::Receive(void* item, uint32_t timeout_ms)
{
  Result result = Result::ERR_NULL_PTR;

  // Check queue handle and item pointer
  if((queue != nullptr) && (item != nullptr))
  {
    // Set new error result
    result = Result::ERR_QUEUE_READ;
    // Result of operation
    portBASE_TYPE res = pdFALSE;

    // Check handler mode
    if(Rtos::IsInHandlerMode())
    {
      if(IsEmpty() == false)
      {
        BaseType_t task_woken;
        // Receive item from the queue from ISR
        res = xQueueReceiveFromISR(queue, item, &task_woken);
        // Switch context if needed
        portEND_SWITCHING_ISR((res == pdPASS) ? task_woken : pdFALSE);
      }
      else
      {
        // Queue is empty - nothing to read
        result = Result::ERR_QUEUE_EMPTY;
      }
    }
    else
    {
      // Receive item from the queue
      res = xQueueReceive(queue, item, RtosTick::MsToTicks(timeout_ms));
      // Check empty error
      if(res == errQUEUE_EMPTY)
      {
        result = Result::ERR_QUEUE_EMPTY;
      }
    }
    // Check result
    if(res == pdPASS)
    {
      result = Result::RESULT_OK;
    }
  }

  return result;
}

// *****************************************************************************
// ***   Peek   ****************************************************************
// *****************************************************************************
Result RtosQueue::Peek(void* item, uint32_t timeout_ms) const
{
  Result result = Result::ERR_QUEUE_READ;

  // Check queue handle and item pointer
  if((queue != nullptr) && (item != nullptr))
  {
    // Result of operation
    portBASE_TYPE res = pdFALSE;

    // Check handler mode
    if(Rtos::IsInHandlerMode())
    {
      // Peek message from ISR
      res = xQueuePeekFromISR(queue, item);
    }
    else
    {
      // Peek message
      res = xQueuePeek(queue, item, RtosTick::MsToTicks(timeout_ms));
    }
    // Check result
    if(res == pdPASS)
    {
      result = Result::RESULT_OK;
    }
  }

  return result;
}
