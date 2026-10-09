#pragma once
#include <string>
#include <vector>
#include "GrblComm.h"
enum { COLOR_GREEN, COLOR_WHITE, COLOR_DARKBLUE, COLOR_RED, COLOR_BLUE, BORDER_W = 2 };
struct Widget {
  bool enabled = true;
  bool IsActive() { return false; }
  void SetNumber(int) {}
  void SetColor(int) {}
  void SetActive(bool) {}
  void SetBorder(int, int) {}
  void SetSelected(bool) {}
  void Enable() { enabled = true; }
  void Disable() { enabled = false; }
};
// Model of the text box: all lines of a program in memory(SetText) or only
// the visible ones of a streamed program(AddLine), with the same rules for
// selection as the real one
struct TextBox {
  static constexpr unsigned MAX_LINE_LEN = 80;
  const char* text = nullptr;
  std::vector<std::string> lines;
  int visible = 4, selection = 0, scroll = 0, added = 0;
  int selector_color = COLOR_RED, selector_fill = 0;
  void SetSelectorColor(int color) { selector_color = color; }
  void SetSelectorFill(uint8_t fill) { selector_fill = fill; }
  void SetText(const char* value) {
    text = value; lines.clear(); selection = scroll = added = 0;
    // Empty lines don't exist for the text box
    for(const char* p = value; p && *p;) {
      while(*p == '\n' || *p == '\r') p++;
      const char* start = p;
      while(*p && *p != '\n' && *p != '\r') p++;
      if(p != start) lines.emplace_back(start, p);
    }
  }
  const char* GetText() { return text; }
  const std::string& Selected() { static const std::string none; return selection < (int)lines.size() ? lines[selection] : none; }
  int GetSelect() { return selection; }
  int GetScroll() { return scroll; }
  int GetNumberOfVisibleLines() { return visible; }
  int GetNumberOfLines() { return text ? (int)lines.size() : added; }
  void Select(int value) { if(value >= 0 && value <= (text ? (int)lines.size() : visible) - 1) selection = value; }
  void Scroll(int value) { scroll = value; }
  void AddLine(const char* value) {
    std::string line(value);
    line.erase(line.find_last_not_of("\r\n") + 1);
    if((int)lines.size() >= visible) lines.erase(lines.begin());
    lines.push_back(line);
    added++;
  }
};
struct Application {
  static Application& GetInstance() { static Application app; return app; }
  void UpdateLeftButtonText() {}
  void UpdateRightButtonText() {}
  void EnableScreenChange() {}
};
// File on SD card: content, read position and the number of lines after
// which reading fails(SD card error)
struct File {
  std::string data;
  size_t pos = 0;
  int fail_after = -1;
  struct { unsigned objsize = 0; } obj;
};
inline bool f_eof(File* file) { return file->pos >= file->data.size(); }
inline char* f_gets(char* buf, unsigned size, File* file) {
  if(f_eof(file) || file->fail_after == 0) return nullptr;
  if(file->fail_after > 0) file->fail_after--;
  unsigned n = 0;
  while(n < size - 1 && file->pos < file->data.size()) {
    buf[n++] = file->data[file->pos++];
    if(buf[n - 1] == '\n') break;
  }
  buf[n] = '\0';
  return buf;
}
inline void f_lseek(File* file, unsigned pos) { file->pos = pos; }
struct MessageBox {
  int shown = 0;
  void Setup(const char*, const char*, unsigned) {}
  void Show(unsigned) { shown++; }
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
  // Sending and showing position, as in ProgramSender.h
  static const uint32_t MAX_LINES_PER_TICK = 8u;
  static const uint32_t MAX_LINE_NUMBER = 9999999u;
  uint32_t send_line = 0u;
  const char* p_send = nullptr;
  char cmd[TextBox::MAX_LINE_LEN + 16u] = {0};
  bool cmd_ready = false;
  uint32_t first_numbered_id = 0u;
  bool line_number_valid = false;
  enum LineMode : uint8_t { LINE_EXECUTED, LINE_SENT };
  // As Run button sets it for a controller that reports line numbers
  LineMode line_mode = LINE_EXECUTED;
  uint32_t exec_line = 0u;
  uint32_t shown_line = 1u;
  File* p_disp_file = nullptr;
  bool disp_end = false;
  // Program in memory, as after Run is pressed
  explicit ProgramSender(GrblComm& comm, const char* program = "G0 X1\nG0 X2\nM2") : grbl_comm(comm), p_text(program) {
    text_box.SetText(p_text);
    p_send = text_box.GetText();
  }
  Result TimerExpired(uint32_t interval);
  bool GetNextLine(char* line, uint32_t size);
  bool ShowNextLine();
  void UpdateShownLine(uint32_t max_lines);
  void ResetSelector();
  static void StripLineNumbers(char* text);
  static bool BuildCommand(const char* line, uint32_t number, char* cmd, uint32_t size);
  void ProcessSpeedFeed() {}
};
