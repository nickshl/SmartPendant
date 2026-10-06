#pragma once
struct NVM {
  enum { TX_CONTROL };
  static NVM& GetInstance() { static NVM instance; return instance; }
  int GetCtrlTx() { return 0; }
  int GetValue(int) { return 0; }
};
