// Double of DevCore for the UsbCdcUart test: time, the critical section and
// nothing else.
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include "Result.h"
#include "IUart.h"
#define NumberOf(a) (sizeof(a) / sizeof((a)[0]))
struct RtosTick {
  static inline uint32_t now = 0;
  static uint32_t GetTimeMs() { return now; }
  static bool CheckTimeDifferenceMs(uint32_t start, uint32_t ms) { return (uint32_t)(now - start) >= ms; }
};
// Critical section: the test checks it is held around every call into the
// stack from the task side. Nesting counts, like taskENTER_CRITICAL().
inline uint32_t sim_critical = 0u;
struct Rtos {
  static void EnterCriticalSection() { sim_critical++; }
  static void ExitCriticalSection() { sim_critical--; }
};
