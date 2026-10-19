// Host test of the settings storage: the real NVM.cpp over a simulated
// 24xx256(stubs_nvm/DevCore.h) with power loss and bus error injection.
//
// The record format is written and parsed here by hand(id, seq, value,
// CRC32 of the first 8 bytes, little endian), not taken from NVM.h, so a
// change of the format fails the test: pendants in the field hold it.
//
// NVM::data[] is static and initialized once per process. Boot() puts it back
// the way it is after the reset and calls what the firmware calls: Init() from
// AppMain() and GetReadResult() from Application::Setup().
#include "NVM.h"
#include <cstdio>
#include <map>
#include <set>
#include <vector>

// Seed of the random test which is running, to repeat a failure: nvm <seed> 1
static uint32_t test_seed = 0u;

#define CHECK(cond) do { if(!(cond)) { printf("%s:%d: check failed: %s (seed %u)\n", __FILE__, __LINE__, #cond, (unsigned)test_seed); exit(1); } } while(0)

static const uint32_t PAGE = 64u;
static const uint32_t BANK = 256u;   // Pages in one bank

// Defaults, saved before anything changes data[]
static std::vector<int32_t> defaults;

static int32_t Default(NVM::Parameters id)
{
  for(uint32_t i = 0; i < NVM::PARAMS_CNT; i++) if(NVM::data[i].id == id) return defaults[i];
  CHECK(false); return 0;
}

// Power on
struct Boot {
  NVM nvm;
  Result read;   // Result of reading(Init)
  Result setup;  // What Application::Setup() gets
  uint32_t reads;
  uint32_t writes;
  explicit Boot(Eeprom24& e) {
    for(uint32_t i = 0; i < NVM::PARAMS_CNT; i++) {
      NVM::data[i].value = defaults[i];
      NVM::data[i].seq = 0u;
      NVM::data[i].page = 0u;
      NVM::data[i].bank = 0u;
      NVM::data[i].valid = 0u;
    }
    uint32_t r0 = e.reads, w0 = e.writes;
    nvm.Init(e);
    read = nvm.read_result;
    // Reading never writes: it runs before the scheduler is started
    CHECK(e.writes == w0);
    reads = e.reads - r0;
    setup = nvm.GetReadResult();
    writes = e.writes - w0;
  }
};

// *** Record as it is in the chip ***
static uint32_t Addr(uint32_t bank, uint32_t page) { return (bank * BANK + page) * PAGE; }

static void PutRecord(Eeprom24& e, uint32_t bank, uint32_t page, uint16_t id, uint16_t seq, int32_t value)
{
  uint8_t* p = &e.mem[Addr(bank, page)];
  uint32_t v = (uint32_t)value;
  p[0] = id & 0xFF; p[1] = id >> 8; p[2] = seq & 0xFF; p[3] = seq >> 8;
  p[4] = v & 0xFF; p[5] = (v >> 8) & 0xFF; p[6] = (v >> 16) & 0xFF; p[7] = v >> 24;
  uint32_t crc = Crc32(p, 8);
  p[8] = crc & 0xFF; p[9] = (crc >> 8) & 0xFF; p[10] = (crc >> 16) & 0xFF; p[11] = crc >> 24;
}

static bool GetRecord(Eeprom24& e, uint32_t bank, uint32_t page, uint16_t& id, uint16_t& seq, int32_t& value)
{
  const uint8_t* p = &e.mem[Addr(bank, page)];
  uint32_t crc = p[8] | (p[9] << 8) | (p[10] << 16) | ((uint32_t)p[11] << 24);
  if(crc != Crc32(p, 8)) return false;
  id = p[0] | (p[1] << 8);
  seq = p[2] | (p[3] << 8);
  value = (int32_t)(p[4] | (p[5] << 8) | (p[6] << 16) | ((uint32_t)p[7] << 24));
  return true;
}

// Page of a setting, as found by the last boot
static uint32_t PageOf(NVM::Parameters id)
{
  for(uint32_t i = 0; i < NVM::PARAMS_CNT; i++) if(NVM::data[i].id == id) return NVM::data[i].page;
  CHECK(false); return 0;
}

// Every setting has its default except the listed ones
static void CheckValues(NVM& nvm, const std::map<int, int32_t>& changed = {})
{
  for(uint32_t i = 0; i < NVM::PARAMS_CNT; i++) {
    NVM::Parameters id = NVM::data[i].id;
    auto it = changed.find(id);
    CHECK(nvm.GetValue(id) == ((it != changed.end()) ? it->second : defaults[i]));
  }
}

// Chip after the first boot
static void Format(Eeprom24& e) { Boot b(e); CHECK(b.setup == Result::ERR_INVALID_DATA); }

static int32_t Rand32() { return (int32_t)((uint32_t)rand() * 2654435761u); }

// ****************************************************************************
// A new chip: defaults, written into both banks, then clean boots that stop
// reading early
static void TestBlankChip()
{
  Eeprom24 e;
  {
    Boot b(e);
    CHECK(b.read.IsGood());
    CHECK(b.setup == Result::ERR_INVALID_DATA);
    CHECK(b.reads == 2 * BANK);
    CHECK(b.writes == 2 * NVM::PARAMS_CNT);
    CheckValues(b.nvm);
  }
  // Every setting in both banks, on its own page, with seq 1 and 2
  std::set<uint32_t> pages;
  for(uint32_t i = 0; i < NVM::PARAMS_CNT; i++) {
    uint32_t page = NVM::data[i].page;
    CHECK(pages.insert(page).second);
    for(uint32_t bank = 0; bank < 2; bank++) {
      uint16_t id, seq; int32_t v;
      CHECK(GetRecord(e, bank, page, id, seq, v));
      CHECK(id == NVM::data[i].id && v == defaults[i] && seq == (bank == 0 ? 2 : 1));
    }
  }
  for(int i = 0; i < 3; i++) {
    Boot b(e);
    CHECK(b.setup.IsGood());
    CHECK(b.writes == 0);
    CHECK(b.reads <= 2 * NVM::PARAMS_CNT);
    CheckValues(b.nvm);
  }
}

// The chip after the firmware which kept all settings in one struct at
// address 0: nothing in it is a valid record
static void TestOldFormat()
{
  static const int32_t old_defaults[26] = {0, 2, 115200, 0, 4, 0, 0, 0, 0, 0, 1, 5, 10, 100, 1, 2, 5, 50, 180, 360, 720, 3600, 200, 50, 10, 2000};
  for(int32_t tx = 0; tx < 4; tx++) {
    Eeprom24 e;
    int32_t old[27];
    memcpy(old, old_defaults, sizeof(old_defaults));
    old[1] = tx; old[2] = 9600;
    old[26] = (int32_t)Crc32((uint8_t*)old, 26 * 4);
    memcpy(&e.mem[0], old, sizeof(old));
    Boot b(e);
    CHECK(b.setup == Result::ERR_INVALID_DATA);
    CheckValues(b.nvm);
  }
}

// Change, power cycle, the value is there. Writes alternate between banks.
static void TestClickPowerCycle()
{
  Eeprom24 e;
  Format(e);
  for(int32_t i = 1; i <= 6; i++) {
    { Boot b(e); CHECK(b.nvm.SetValue(NVM::PROBE_BALL_TIP, 1000 + i).IsGood()); }
    Boot b(e);
    CHECK(b.setup.IsGood());
    CheckValues(b.nvm, {{NVM::PROBE_BALL_TIP, 1000 + i}});
  }
  // Several changes in one session: the last one is there, one page write
  // each, alternating
  uint32_t page = PageOf(NVM::PROBE_BALL_TIP);
  uint32_t w0 = e.wear[page], w1 = e.wear[BANK + page];
  {
    Boot b(e);
    for(int32_t i = 1; i <= 7; i++) {
      uint32_t writes = e.writes;
      CHECK(b.nvm.SetValue(NVM::BAUD_RATE, 1000 * i).IsGood());
      CHECK(e.writes == writes + 1);
      CHECK(b.nvm.GetValue(NVM::BAUD_RATE) == 1000 * i);
      CHECK(b.nvm.SetValue(NVM::PROBE_BALL_TIP, 2000 + i).IsGood());
      uint32_t d = (e.wear[page] - w0) - (e.wear[BANK + page] - w1);
      CHECK(d == 0 || d == 1 || d == (uint32_t)-1);
    }
  }
  Boot b(e);
  CheckValues(b.nvm, {{NVM::BAUD_RATE, 7000}, {NVM::PROBE_BALL_TIP, 2007}});
  // Only the pages of these two settings were written after the first boot
  uint32_t baud = PageOf(NVM::BAUD_RATE);
  for(uint32_t p = 0; p < 2 * BANK; p++) {
    if((p % BANK == page) || (p % BANK == baud)) continue;
    CHECK(e.wear[p] <= 1);
  }
}

// Power is lost during a change, with every kind of page damage: the setting
// has the old or the new value, never the default, and nothing else changes
static void TestPowerLossOnChange()
{
  for(int tear = 0; tear < Eeprom24::TEAR_CNT; tear++) for(int changes = 1; changes <= 4; changes++) {
    Eeprom24 e;
    Format(e);
    int32_t stored = Default(NVM::PROBE_BALL_TIP);
    {
      Boot b(e);
      for(int i = 1; i < changes; i++) { CHECK(b.nvm.SetValue(NVM::PROBE_BALL_TIP, 3000 + i).IsGood()); stored = 3000 + i; }
      e.fault_at = e.writes + 1; e.power_loss = true; e.tear = (Eeprom24::Tear)tear;
      b.nvm.SetValue(NVM::PROBE_BALL_TIP, 3999);
      CHECK(!e.powered);
      e.fault_at = 0; e.power_loss = false; e.powered = true;
    }
    Boot b(e);
    CHECK(b.setup.IsGood());
    int32_t v = b.nvm.GetValue(NVM::PROBE_BALL_TIP);
    if(tear == Eeprom24::TEAR_COMPLETE) CHECK(v == 3999);
    else if(tear == Eeprom24::TEAR_PARTIAL) CHECK(v == stored || v == 3999);
    else CHECK(v == stored);
    CheckValues(b.nvm, {{NVM::PROBE_BALL_TIP, v}});
    // And the next change goes where it should
    CHECK(b.nvm.SetValue(NVM::PROBE_BALL_TIP, 4444).IsGood());
    Boot b2(e);
    CheckValues(b2.nvm, {{NVM::PROBE_BALL_TIP, 4444}});
  }
}

// Power is lost during each write of the first boot
static void TestPowerLossOnFirstBoot()
{
  for(int tear = 0; tear < Eeprom24::TEAR_CNT; tear++) for(uint32_t cut = 1; cut <= 2 * NVM::PARAMS_CNT; cut++) {
    Eeprom24 e;
    e.fault_at = cut; e.power_loss = true; e.tear = (Eeprom24::Tear)tear;
    { Boot b(e); CHECK(!e.powered); }
    e.fault_at = 0; e.power_loss = false; e.powered = true;
    {
      Boot b(e);
      CHECK(b.read.IsGood());
      CHECK(b.setup.IsGood() || b.setup == Result::ERR_INVALID_DATA);
      CheckValues(b.nvm);
    }
    Boot b(e);
    CHECK(b.setup.IsGood());
    CHECK(b.writes == 0);
    CheckValues(b.nvm);
  }
}

// The chip doesn't answer at boot: the error is reported, nothing is written
static void TestReadError()
{
  // A clean boot reads 2 * PARAMS_CNT records: fail one of them
  for(uint32_t at = 1; at <= 2 * NVM::PARAMS_CNT; at += 7) {
    Eeprom24 e;
    Format(e);
    { Boot b(e); CHECK(b.nvm.SetValue(NVM::BAUD_RATE, 9600).IsGood()); CHECK(b.nvm.SetValue(NVM::BAUD_RATE, 19200).IsGood()); }
    e.fail_read_at = e.reads + at;
    {
      Boot b(e);
      CHECK(b.read == Result::ERR_I2C_GENERAL);
      CHECK(b.setup == Result::ERR_I2C_GENERAL);
      CHECK(b.writes == 0);
    }
    e.fail_read_at = 0;
    Boot b(e);
    CHECK(b.setup.IsGood());
    CheckValues(b.nvm, {{NVM::BAUD_RATE, 19200}});
  }
}

// A write fails: the old value stays and the error is returned. A power loss
// during the next change still leaves the last stored value.
static void TestWriteError()
{
  for(int written = 0; written < 2; written++) for(int tear = Eeprom24::TEAR_GARBAGE; tear <= Eeprom24::TEAR_ZEROED; tear++) {
    Eeprom24 e;
    Format(e);
    {
      Boot b(e);
      CHECK(b.nvm.SetValue(NVM::PROBE_BALL_TIP, 1111).IsGood());
      e.fault_at = e.writes + 1; e.error_no_write = !written; e.error_but_written = written;
      CHECK(b.nvm.SetValue(NVM::PROBE_BALL_TIP, 2222).IsBad());
      e.fault_at = 0; e.error_no_write = e.error_but_written = false;
      CHECK(b.nvm.GetValue(NVM::PROBE_BALL_TIP) == 1111);
      e.fault_at = e.writes + 1; e.power_loss = true; e.tear = (Eeprom24::Tear)tear;
      b.nvm.SetValue(NVM::PROBE_BALL_TIP, 3333);
      e.fault_at = 0; e.power_loss = false; e.powered = true;
    }
    Boot b(e);
    CHECK(b.setup.IsGood());
    int32_t v = b.nvm.GetValue(NVM::PROBE_BALL_TIP);
    CHECK(v == 1111 || (written && v == 2222));
  }
}

// The sequence number wraps around: one change per power cycle near the wrap
static void TestSequenceWrap()
{
  Eeprom24 e;
  Format(e);
  Boot* b = new Boot(e);
  for(int32_t i = 1; i <= 65700; i++) {
    CHECK(b->nvm.SetValue(NVM::MPG_METRIC_FEED_1, 10000 + i).IsGood());
    if((i > 65520) || (i % 4999 == 0)) {
      delete b;
      b = new Boot(e);
      CHECK(b->setup.IsGood());
      CHECK(b->nvm.GetValue(NVM::MPG_METRIC_FEED_1) == 10000 + i);
    }
  }
  delete b;
}

// Records with an ID the firmware doesn't know(a deleted setting): ignored,
// and a setting without a record takes such a page
static void TestPageReuse()
{
  Eeprom24 e;
  Format(e);
  { Boot b(e); CHECK(b.nvm.SetValue(NVM::BAUD_RATE, 9600).IsGood()); }
  // SCREEN_INVERT "is new": its page holds records of a deleted setting
  uint32_t page = PageOf(NVM::SCREEN_INVERT);
  PutRecord(e, 0, page, 200, 5, 777);
  PutRecord(e, 1, page, 200, 4, 776);
  {
    Boot b(e);
    CHECK(b.setup == Result::ERR_INVALID_DATA);
    CHECK(b.writes == 2);
    CHECK(PageOf(NVM::SCREEN_INVERT) == page);
    CheckValues(b.nvm, {{NVM::BAUD_RATE, 9600}});
  }
  Boot b(e);
  CHECK(b.setup.IsGood());
  CheckValues(b.nvm, {{NVM::BAUD_RATE, 9600}});
  // Unknown records elsewhere don't disturb anything
  PutRecord(e, 0, 200, 300, 1, 1);
  PutRecord(e, 1, 201, 301, 1, 1);
  Boot b2(e);
  CHECK(b2.setup.IsGood());
  CheckValues(b2.nvm, {{NVM::BAUD_RATE, 9600}});
}

// Random changes, power cycles, power losses and bus errors against a model.
// A value can come back after a power cycle if it was written into a page that
// hasn't been fully overwritten since - the last stored one, the one of the
// change the fault happened in, or an older one: a write reported as failed
// can still reach the chip, and two torn writes into that page can leave its
// bytes as they were. Two successful writes in a row overwrite both pages, and
// from then on only the last value is possible.
static void TestRandom(uint32_t seed)
{
  test_seed = seed;
  srand(seed);
  static const int32_t some[] = {0, 1, 2, 5, 100, 9600, 115200, -1, INT32_MIN, INT32_MAX};
  Eeprom24 e;
  Format(e);
  struct Model { std::set<int32_t> possible; int clean = 2; };
  std::map<int, Model> model;
  for(uint32_t i = 0; i < NVM::PARAMS_CNT; i++) model[NVM::data[i].id].possible = {defaults[i]};
  auto stored = [&](int id, int32_t v) {
    Model& m = model[id];
    m.possible.insert(v);
    if(++m.clean >= 2) m.possible = {v};
  };
  auto fault = [&](int id, int32_t v) { model[id].possible.insert(v); model[id].clean = 0; };
  Boot* b = new Boot(e);
  auto restart = [&]() {
    delete b;
    b = new Boot(e);
    CHECK(b->setup.IsGood());
    for(auto& m : model) {
      int32_t v = b->nvm.GetValue((NVM::Parameters)m.first);
      CHECK(m.second.possible.count(v) == 1);
      // Without a fault since two writes, it is exactly the last value
      if(m.second.clean >= 2) CHECK(m.second.possible.size() == 1);
    }
  };
  for(int op = 0; op < 300; op++) {
    NVM::Parameters id = NVM::data[rand() % NVM::PARAMS_CNT].id;
    int32_t v = (rand() % 3) ? some[rand() % NumberOf(some)] : Rand32();
    int k = rand() % 100;
    if(k < 70) {
      CHECK(b->nvm.SetValue(id, v).IsGood());
      CHECK(b->nvm.GetValue(id) == v);
      stored(id, v);
    } else if(k < 80) {
      restart();
    } else if(k < 90) {
      e.fault_at = e.writes + 1; e.power_loss = true; e.tear = (Eeprom24::Tear)(rand() % Eeprom24::TEAR_CNT);
      b->nvm.SetValue(id, v);
      e.fault_at = 0; e.power_loss = false; e.powered = true;
      fault(id, v);
      restart();
    } else {
      bool w = rand() % 2;
      int32_t before = b->nvm.GetValue(id);
      e.fault_at = e.writes + 1; e.error_no_write = !w; e.error_but_written = w;
      CHECK(b->nvm.SetValue(id, v).IsBad());
      e.fault_at = 0; e.error_no_write = e.error_but_written = false;
      // The value in RAM stays as it was
      CHECK(b->nvm.GetValue(id) == before);
      fault(id, v);
    }
  }
  delete b;
}

int main(int argc, char** argv)
{
  // First seed and number of seeds of the random test
  uint32_t seed = (argc > 1) ? strtoul(argv[1], nullptr, 0) : 1u;
  uint32_t count = (argc > 2) ? strtoul(argv[2], nullptr, 0) : 3000u;
  for(uint32_t i = 0; i < NVM::PARAMS_CNT; i++) defaults.push_back(NVM::data[i].value);
  TestBlankChip();
  TestOldFormat();
  TestClickPowerCycle();
  TestPowerLossOnChange();
  TestPowerLossOnFirstBoot();
  TestReadError();
  TestWriteError();
  TestSequenceWrap();
  TestPageReuse();
  for(uint32_t s = seed; s < seed + count; s++) TestRandom(s);
  printf("NVM: all checks passed, random seeds %u..%u\n", (unsigned)seed, (unsigned)(seed + count - 1));
  return 0;
}
