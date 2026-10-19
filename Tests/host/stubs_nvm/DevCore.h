// Doubles of DevCore for the host test of NVM: a 24xx256 EEPROM with pages,
// wear counters and fault injection. NVM uses nothing else from DevCore.
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <vector>
#include "Result.h"
#include "Crc32.h"
#define NumberOf(a) (sizeof(a) / sizeof((a)[0]))

class Eeprom24 {
public:
  // What is left in the page when the power is lost during its write cycle
  enum Tear { TEAR_NOT_STARTED, TEAR_GARBAGE, TEAR_ERASED, TEAR_ZEROED, TEAR_PARTIAL, TEAR_COMPLETE, TEAR_CNT };

  explicit Eeprom24(uint32_t size = 0x8000u, uint32_t page = 64u)
      : mem(size, 0xFFu), wear(size / page, 0u), page_size(page) {}

  Result Read(uint16_t addr, uint8_t* buf, uint16_t size) {
    reads++;
    if(fail_read_at && reads == fail_read_at) return Result::ERR_I2C_GENERAL;
    if(buf == nullptr) return Result::ERR_NULL_PTR;
    if((uint32_t)addr + size > mem.size()) return Result::ERR_INVALID_SIZE;
    if(fail_reads || !powered) return Result::ERR_I2C_GENERAL;
    memcpy(buf, &mem[addr], size);
    return Result::RESULT_OK;
  }

  // Splits data into pages the same way as the real driver does
  Result Write(uint16_t addr, uint8_t* buf, uint16_t size) {
    if(buf == nullptr) return Result::ERR_NULL_PTR;
    if((uint32_t)addr + size > mem.size()) return Result::ERR_INVALID_SIZE;
    while(size) {
      uint32_t chunk = page_size - (addr % page_size);
      if(chunk > size) chunk = size;
      Result result = WritePage(addr, buf, chunk);
      if(result.IsBad()) return result;
      addr += chunk; buf += chunk; size -= chunk;
    }
    return Result::RESULT_OK;
  }

  uint32_t GetSize() { return mem.size(); }
  uint32_t GetPageSize() { return page_size; }

  // *** Test side ***
  std::vector<uint8_t> mem;      // Content of the chip
  std::vector<uint32_t> wear;    // Write cycles of every page
  uint32_t page_size;
  uint32_t writes = 0;           // Page writes requested so far
  uint32_t reads = 0;
  bool fail_reads = false;       // Bus error on every read
  uint32_t fail_read_at = 0;     // Read number(counted in reads) to fail once
  // Page write number(counted from 1 in "writes") to fail, 0 - none.
  // The fault is one of the modes below.
  uint32_t fault_at = 0;
  Tear tear = TEAR_GARBAGE;      // Used with power_loss
  // Power is lost during the write. The firmware code can't be stopped in
  // the middle, so the chip stops instead: "powered" becomes false and
  // nothing the code does after that reaches the chip. The test checks the
  // flag, drops the NVM object and sets the flag back - it is the restart.
  bool power_loss = false;
  bool powered = true;
  bool error_no_write = false;   // Device doesn't answer: error, page is intact
  bool error_but_written = false;// Page is written, but error is reported
  bool fail_writes = false;      // Device doesn't answer to any write

private:
  Result WritePage(uint16_t addr, const uint8_t* buf, uint32_t size) {
    uint32_t page = addr / page_size;
    if(!powered) return Result::ERR_I2C_TIMEOUT;
    writes++;
    if(fail_writes) return Result::ERR_I2C_TIMEOUT;
    bool fault = (fault_at != 0u) && (writes == fault_at);
    if(fault && error_no_write) return Result::ERR_I2C_GENERAL;
    // From this point the write cycle is started and the page is worn
    wear[page]++;
    if(fault && power_loss) {
      uint8_t* p = &mem[page * page_size];
      switch(tear) {
        case TEAR_NOT_STARTED: wear[page]--; break;
        case TEAR_GARBAGE: for(uint32_t i = 0; i < page_size; i++) p[i] = (uint8_t)rand(); break;
        case TEAR_ERASED: memset(p, 0xFF, page_size); break;
        case TEAR_ZEROED: memset(p, 0x00, page_size); break;
        // Some bytes are new, the rest of the written range keeps old data
        case TEAR_PARTIAL: memcpy(&mem[addr], buf, (size > 1u) ? (1u + (uint32_t)rand() % (size - 1u)) : 0u); break;
        // Data is in the chip, the power is lost before the firmware knows it
        case TEAR_COMPLETE: memcpy(&mem[addr], buf, size); break;
        default: break;
      }
      powered = false;
      return Result::ERR_I2C_TIMEOUT;
    }
    memcpy(&mem[addr], buf, size);
    if(fault && error_but_written) return Result::ERR_I2C_TIMEOUT;
    return Result::RESULT_OK;
  }
};
