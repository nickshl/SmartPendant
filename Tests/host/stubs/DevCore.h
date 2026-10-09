#pragma once
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <deque>
#include <vector>
#include "Result.h"
#include "IUart.h"
#include "Decimal32.h"
#define NumberOf(a) (sizeof(a) / sizeof((a)[0]))
#define APPLICATION_TASK_STACK_SIZE 1536u
#define APPLICATION_TASK_PRIORITY 3u
#define GRBLCOMM_TASK_STACK_SIZE 512u
#define GRBLCOMM_TASK_PRIORITY 3u
#define MPG_EN_GPIO_Port 0
#define MPG_EN_Pin 0
#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0
inline void HAL_GPIO_WritePin(int, int, int) {}
struct RtosTick {
  static inline uint32_t now = 0;
  static uint32_t GetTimeMs() { return now; }
  static void DelayMs(uint32_t ms) { now += ms; }
};
// Not recursive, as the real one: locking it twice would hang the task
struct RtosMutex {
  bool locked = false;
  void Lock() { assert(!locked); locked = true; }
  void Release() { assert(locked); locked = false; }
};
class AppTask {
public:
  std::deque<std::vector<uint8_t>> messages;
  size_t message_size;
  AppTask(unsigned, unsigned, const char*, unsigned, size_t size, void*, unsigned, bool)
      : message_size(size) {}
  Result InitTask() { return Result::RESULT_OK; }
  Result SendTaskMessage(void* message, bool front = false) {
    auto first = static_cast<uint8_t*>(message);
    std::vector<uint8_t> copy(first, first + message_size);
    if(front) messages.push_front(copy); else messages.push_back(copy);
    return Result::RESULT_OK;
  }
};
