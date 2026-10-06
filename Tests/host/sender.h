#pragma once
#include "GrblComm.h"
enum { COLOR_GREEN, COLOR_WHITE, COLOR_DARKBLUE, BORDER_W = 2 };
struct Widget {
  bool IsActive() { return false; }
  void SetNumber(int) {}
  void SetColor(int) {}
  void SetActive(bool) {}
  void SetBorder(int, int) {}
  void SetSelected(bool) {}
  void Enable() {}
  void Disable() {}
};
struct TextBox {
  static constexpr unsigned MAX_LINE_LEN = 80;
  int selection = 0;
  const char* GetSelectedStringText() { return "G0 X1"; }
  int GetSelect() { return selection; }
  int GetScroll() { return 0; }
  int GetNumberOfVisibleLines() { return 10; }
  int GetNumberOfLines() { return 3; }
  void Select(int value) { if(value < 3) selection = value; }
  void Scroll(int) {}
  void AddLine(const char*) {}
};
struct Application {
  static Application& GetInstance() { static Application app; return app; }
  void UpdateLeftButtonText() {}
  void UpdateRightButtonText() {}
  void EnableScreenChange() {}
};
struct File { struct { unsigned objsize = 0; } obj; };
inline bool f_eof(File*) { return true; }
inline char* f_gets(char*, unsigned, File*) { return nullptr; }
inline void f_lseek(File*, unsigned) {}
struct MessageBox {
  void Setup(const char*, const char*, unsigned) {}
  void Show(unsigned) {}
};
struct ProgramSender {
  GrblComm& grbl_comm;
  bool run = true, finished = false;
  uint32_t id = 1;
  int enc_val = 0;
  const char* p_text = "G0 X1\nG0 X2\nM2";
  TextBox text_box;
  File SDFile;
  Widget feed_dw, speed_dw, flood_btn, mist_btn, left_btn, middle_btn;
  MessageBox msg_box;
  explicit ProgramSender(GrblComm& comm) : grbl_comm(comm) {}
  Result TimerExpired(uint32_t interval);
  void ProcessSpeedFeed() {}
};
