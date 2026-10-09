#include <cassert>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include "GrblComm.h"
#include "FramedUart.h"
#include "Little-C.h"
#include "sender.h"

struct FakeUart : IUart {
  std::deque<uint8_t> incoming;
  std::vector<std::vector<uint8_t>> outgoing;
  Result write_result = Result::RESULT_OK;
  Result Init() override { return Result::RESULT_OK; }
  Result SetBaudRate(uint32_t) override { return Result::RESULT_OK; }
  Result Read(uint8_t& byte) override {
    if(incoming.empty()) return Result::ERR_UART_EMPTY;
    byte = incoming.front(); incoming.pop_front(); return Result::RESULT_OK;
  }
  Result Write(uint8_t* data, uint32_t size) override {
    if(write_result.IsGood()) outgoing.emplace_back(data, data + size);
    return write_result;
  }
  void frame(uint8_t seq, mpg_frame_type_t type, const std::vector<uint8_t>& payload) {
    uint8_t bytes[MPG_TRANSPORT_MAX_FRAME];
    size_t size = mpg_frame_build(bytes, seq, type, payload.data(), payload.size());
    incoming.insert(incoming.end(), bytes, bytes + size);
  }
  void line(const std::string& text) { incoming.insert(incoming.end(), text.begin(), text.end()); }
};

void setup(GrblComm& comm, IUart& uart) {
  comm.uart = &uart;
  comm.grbl_state = GrblComm::IDLE;
  comm.grbl_status = GrblComm::Status_OK;
  comm.grbl_mpgMode = true;
  comm.request_settings = false;
}
void dispatch(GrblComm& comm) {
  assert(!comm.messages.empty());
  memcpy(&comm.rcv_msg, comm.messages.front().data(), sizeof(comm.rcv_msg));
  comm.messages.pop_front();
  assert(comm.ProcessMessage().IsGood());
}

void comm_tests() {
  FakeUart wire;
  FramedUart framed(wire);
  framed.Init();
  GrblComm comm;
  setup(comm, framed);
  RtosTick::now = 0;
  uint32_t first = 0, second = 0;
  assert(comm.SendCmd("G0 X1\r", first).IsGood()); dispatch(comm);
  // Controller executed the line, but its transport ACK is delayed/lost.
  wire.frame(0, MpgFrame_Dat, {'o', 'k', '\n'}); comm.PollSerial();
  assert(comm.GetCmdResult(first) == GrblComm::Status_OK);
  assert(comm.SendCmd("G0 X2\r", second).IsGood()); dispatch(comm);
  assert(!comm.respond_pending && comm.send_id == first);
  assert(comm.GetCmdResult(second) == GrblComm::Status_Cmd_Not_Executed_Yet);
  // Priority stop/hold still gets through while the command retries.
  assert(comm.SendRealTimeCmd('!').IsGood()); dispatch(comm);
  wire.frame(0, MpgFrame_Ack, {MpgFrame_Dat}); comm.PollSerial();
  dispatch(comm);
  assert(comm.respond_pending && comm.send_id == second);
  wire.frame(1, MpgFrame_Dat, {'o', 'k', '\n'}); comm.PollSerial();
  assert(comm.GetCmdResult(second) == GrblComm::Status_OK);
  int commands = 0;
  for(auto& bytes : wire.outgoing) if(bytes[2] == MpgFrame_Dat) commands++;
  assert(commands == 2);

  FakeUart raw;
  GrblComm plain;
  setup(plain, raw);
  uint32_t id = 0;
  assert(plain.SendCmd("G0 X3\r", id).IsGood()); dispatch(plain);
  RtosTick::now = 301;
  plain.TimerExpired(0);
  assert(plain.send_id == id && !plain.respond_pending);
  assert(plain.GetCmdResult(id) == GrblComm::Status_Comm_Error);
  raw.line("ok\n"); plain.PollSerial();
  assert(plain.GetCmdResult(id) == GrblComm::Status_Comm_Error);
  assert(!plain.IsStatusReceivedAfterCmd(id));
  plain.send_id++;
  assert(!plain.IsStatusReceivedAfterCmd(id));

  GrblComm failed;
  setup(failed, raw);
  raw.write_result = Result::ERR_UART_TRANSMIT;
  assert(failed.SendCmd("G0 X4\r", id).IsGood()); dispatch(failed);
  assert(!failed.respond_pending && failed.GetCmdResult(id) == GrblComm::Status_Comm_Error);
  std::cout << "Communication fault tests passed\n";
}

// Is the last status newer than a command: it has to be requested after it
void status_after_cmd_tests() {
  FakeUart raw;
  GrblComm comm;
  setup(comm, raw);
  RtosTick::now = 1000;
  auto send = [&](const char* cmd) { uint32_t id = 0; RtosTick::now++; assert(comm.SendCmd(cmd, id).IsGood()); dispatch(comm); return id; };
  auto respond = [&](const char* text) { RtosTick::now++; raw.line(text); comm.PollSerial(); };
  auto request = [&]() { RtosTick::now++; comm.status_tx_timestamp = RtosTick::now; };
  auto answer = [&]() { RtosTick::now++; raw.line("<Idle|WPos:0,0,0>\n"); comm.PollSerial(); };

  // The last command sent
  request();
  uint32_t a = send("G0 X1\r");
  assert(!comm.IsStatusReceivedAfterCmd(a));           // Not acknowledged
  respond("ok\n");
  answer();                                            // Received after the command, but requested before it
  assert(comm.GetCmdResult(a) == GrblComm::Status_OK && !comm.IsStatusReceivedAfterCmd(a));
  request();
  assert(!comm.IsStatusReceivedAfterCmd(a));           // No answer yet
  answer();
  assert(comm.IsStatusReceivedAfterCmd(a));
  request();                                           // The next request changes nothing
  assert(comm.IsStatusReceivedAfterCmd(a));
  answer();
  assert(comm.IsStatusReceivedAfterCmd(a));

  // A command followed by the next one: status has to be requested after the next one was sent
  request();
  uint32_t b = send("G0 X2\r");
  assert(comm.GetCmdResult(a) == GrblComm::Status_Next_Cmd_Executed && !comm.IsStatusReceivedAfterCmd(a));
  answer();
  assert(!comm.IsStatusReceivedAfterCmd(a));
  request(); answer();
  assert(comm.IsStatusReceivedAfterCmd(a) && !comm.IsStatusReceivedAfterCmd(b));
  respond("ok\n");
  request();
  assert(comm.IsStatusReceivedAfterCmd(a) && !comm.IsStatusReceivedAfterCmd(b));
  answer();
  assert(comm.IsStatusReceivedAfterCmd(a) && comm.IsStatusReceivedAfterCmd(b));

  // A command that wasn't sent closes the question for everything before it
  uint32_t c = send("G0 X3\r");
  respond("error:20\n");
  uint32_t d = send("G0 X4\r");                        // Dropped: the previous one has failed
  request(); answer();
  assert(comm.GetCmdResult(b) == GrblComm::Status_Next_Cmd_Executed);
  for(uint32_t id : {a, b, c, d}) assert(!comm.IsStatusReceivedAfterCmd(id));

  // So does Stop
  comm.Stop(); dispatch(comm);
  uint32_t e = send("G0 X5\r");
  respond("ok\n");
  uint32_t f = send("G0 X6\r");
  request(); answer();
  assert(comm.IsStatusReceivedAfterCmd(e));
  comm.Stop(); dispatch(comm);
  request(); answer();
  for(uint32_t id : {a, b, c, d, e, f}) assert(!comm.IsStatusReceivedAfterCmd(id));

  // And the link that was lost and is back
  uint32_t g = send("G0 X7\r");
  RtosTick::now += 301;
  comm.TimerExpired(0);
  assert(comm.GetCmdResult(g) == GrblComm::Status_Comm_Error);
  request(); answer();
  assert(comm.GetCmdResult(g) == GrblComm::Status_Next_Cmd_Executed && comm.abort_id > g && !comm.IsStatusReceivedAfterCmd(g));

  // And control that is given away
  uint32_t next = comm.next_id;
  comm.mpg_mode_request = false;
  comm.ReleaseControl();
  assert(comm.abort_id == next && comm.send_id == next);
  std::cout << "Status after command tests passed\n";
}

void probe_tests() {
  GrblComm comm;
  FakeUart raw;
  setup(comm, raw);
  comm.number_of_axis = 3;
  auto parse = [&](const std::string& report) { raw.line(report + "\n"); comm.PollSerial(); };
  parse("[PRB:1,2,3:1]");
  assert(comm.IsProbeDataReceived() && comm.IsProbeSucceed());
  parse("[PRB:1,2,3:1]"); // Same position is still a fresh report.
  assert(comm.IsProbeDataReceived() && comm.IsProbeSucceed());
  for(const auto& bad : {"[PRB:12.3:1]", "[PRB:1,2:1]", "[PRB:1,2,3,4:1]",
      "[PRB:1,garbage,3:1]", "[PRB:1,nan,3:1]", "[PRB:1,inf,3:1]",
      "[PRB:1,1e99,3:1]", "[PRB:1,2,3]", "[PRB:1,2,3:10]", "[PRB:1,2,3:1",
      "[PRB:1,2,3:1]junk", "[PRB:]", "[PRB:1,2,3:]", "[PRB:1,2,3:1x]"}) {
    parse(bad);
    assert(!comm.IsProbeDataReceived() && !comm.IsProbeSucceed());
    assert(comm.grbl_probe_position[0] == Decimal32(1) && comm.grbl_probe_position[1] == Decimal32(2) && comm.grbl_probe_position[2] == Decimal32(3));
  }
  parse("[PRB:-1.25,0,3.5:0]");
  assert(comm.IsProbeDataReceived() && !comm.IsProbeSucceed());
  assert(comm.grbl_probe_position[0] == Decimal32(-125, 100));
  // Exercise complete malformed buffers, not only zero-padded UART storage.
  std::mt19937 rng(std::random_device{}());
  for(int test = 0; test < 10000; test++) {
    std::string text;
    for(unsigned n = rng() % 60; n; n--) text += "0123456789.,:]-+nifaex"[rng() % 20];
    std::vector<char> data(text.begin(), text.end()); data.push_back(0);
    comm.ParseProbeReport(data.data());
  }
  std::cout << "Probe validation and randomized bounds tests passed\n";
}

void sender_tests() {
  for(auto status : {GrblComm::Status_OK, GrblComm::Status_Comm_Error,
                     GrblComm::Status_Next_Cmd_Executed, GrblComm::Status_Cmd_Not_Executed_Yet}) {
    FakeUart wire;
    GrblComm comm;
    setup(comm, wire);
    comm.send_id = 1;
    comm.next_id = 3;
    comm.grbl_status = status;
    if(status == GrblComm::Status_Next_Cmd_Executed) comm.send_id = 2;
    if(status == GrblComm::Status_Cmd_Not_Executed_Yet) comm.respond_pending = true;
    ProgramSender sender(comm);
    sender.TimerExpired(1);
    if(status == GrblComm::Status_OK) {
      assert(sender.run && comm.messages.size() == 1 && sender.send_line == 1);
    } else {
      assert(comm.messages.empty() && sender.send_line == 0);
      assert(sender.run == (status == GrblComm::Status_Cmd_Not_Executed_Yet));
    }
    // Selection follows what controller reports, not what is sent
    assert(sender.text_box.selection == 0);
  }
  // Run is enabled only after an error of the previous command is cleared
  {
    FakeUart wire;
    GrblComm comm;
    setup(comm, wire);
    ProgramSender sender(comm);
    sender.run = false;
    comm.grbl_status = GrblComm::Status_GcodeUnsupportedCommand;
    sender.TimerExpired(1);
    assert(!sender.left_btn.enabled);
    comm.Stop();
    sender.TimerExpired(1);
    assert(sender.left_btn.enabled);
    // A message added line by line(file was refused) isn't a program
    sender.text_box.SetText(nullptr);
    sender.text_box.AddLine("; Line 7 is longer");
    sender.TimerExpired(1);
    assert(!sender.left_btn.enabled);
  }
  std::cout << "Program streaming acknowledgement tests passed\n";
}

// Program sender with a controller on the other end of the wire
struct Rig {
  FakeUart wire;
  GrblComm comm;
  ProgramSender sender;
  std::vector<std::string> sent;
  explicit Rig(const char* program) : sender(comm, program) {
    setup(comm, wire);
    sender.id = 0; // Nothing is sent yet
  }
  // One timer tick. If a command went out it is recorded and acknowledged.
  bool tick() {
    sender.TimerExpired(1);
    if(comm.messages.empty()) return false;
    dispatch(comm);
    sent.emplace_back(wire.outgoing.back().begin(), wire.outgoing.back().end());
    wire.line("ok\n"); comm.PollSerial();
    return true;
  }
  // Status request and the report that answers it. Time goes on by default:
  // request is later than the last "ok" and report is later than request.
  void request(uint32_t ms = 1u) { RtosTick::now += ms; comm.status_tx_timestamp = RtosTick::now; }
  void answer(const std::string& fields, uint32_t ms = 1u) { RtosTick::now += ms; wire.line("<" + fields + ">\n"); comm.PollSerial(); }
  void report(const std::string& fields) { request(); answer(fields); }
};

std::string strip_numbers(const std::string& text) {
  std::vector<char> buf(text.begin(), text.end()); buf.push_back(0);
  ProgramSender::StripLineNumbers(buf.data());
  return buf.data();
}

void line_number_tests() {
  // Program's own line numbers are removed, nothing else is
  assert(strip_numbers("N10 G1 X1") == "G1 X1" && strip_numbers("n5G0X0") == "G0X0");
  assert(strip_numbers("G1 X1 N20 Y2") == "G1 X1 Y2" && strip_numbers("G1X1N20Y2") == "G1X1Y2");
  assert(strip_numbers("/N5 G1 X1") == "/G1 X1");
  for(const char* same : {"G1 X1 ; N5 stays", "(N5) G1 (N7)", "G1 X1 (unclosed N5", "$N0=G54", "  $N1=G20", "#<n1>=5 o<pin2> call",
                          "RETURN ENDIF LN[2] N", "G1 X1 Y2", ""})
    assert(strip_numbers(same) == same);
  assert(strip_numbers("(N5) N6 G1 (N7)") == "(N5) G1 (N7)");
  assert(strip_numbers("N1 G0\r\nN2 G1 ; N3\nN4\n$N1=G20\n(a\nN5 M2") == "G0\r\nG1 ; N3\n\n$N1=G20\n(a\nM2");

  // Command is the line without comments, with its number in front
  auto build = [](const std::string& line, uint32_t number) {
    char cmd[TextBox::MAX_LINE_LEN + 16u];
    return std::string(ProgramSender::BuildCommand(line.c_str(), number, cmd, sizeof(cmd)) ? cmd : "nothing");
  };
  assert(build("G1 X1 F100", 7) == "N7G1 X1 F100\r" && build("G1 X1\r\n", 9) == "N9G1 X1\r");
  assert(build("  G1 X1 ; comment", 7) == "N7G1 X1\r" && build("M70; Save modal state", 1) == "N1M70\r");
  // Comments in parentheses stay: controller handles them. ';' inside them isn't a comment.
  assert(build("G1 (a) X1 (b ; c) Y2 ; d", 12345) == "N12345G1 (a) X1 (b ; c) Y2\r");
  assert(build("G1 X1 (unclosed", 4) == "N4G1 X1 (unclosed\r");
  assert(build("(MSG, Insert tool 3)", 8) == "N8(MSG, Insert tool 3)\r" && build("(only) ", 3) == "N3(only)\r");
  assert(build("/G1 X1", 3) == "/N3G1 X1\r"); // Block delete character stays first
  for(const char* empty : {"; only comment", "", "   ", "/", "/ ; x", "\r\n"}) assert(build(empty, 3) == "nothing");
  // System commands and the program mark aren't g-code: no number, nothing removed
  assert(build("$H ; text", 5) == "$H ; text\r" && build("%", 5) == "%\r" && build("[ESP800]", 5) == "[ESP800]\r");
  // The longest line with the longest number fits. Controller has a limit for the number: no number above it.
  assert(build("/" + std::string(79, 'X'), 9999999u) == "/N9999999" + std::string(79, 'X') + "\r");
  assert(build("(" + std::string(78, 'X') + ")", 9999999u) == "N9999999(" + std::string(78, 'X') + ")\r");
  assert(build("G1 X1", 10000000u) == "G1 X1\r" && build("; x", 10000000u) == "nothing");

  // Program in memory, controller reports line numbers
  {
    std::string program = strip_numbers("N10 G21 ; metric\n; setup\nN30 G1 X1 F100\n\nG1 X2\n$G\nG1 X3 (last)\n");
    Rig r(program.c_str());
    auto& box = r.sender.text_box;
    assert(r.tick() && r.tick() && box.selection == 0);
    r.report("Run|WPos:0,0,0|Ln:1");
    assert(r.tick() && box.selection == 0);
    r.report("Run|WPos:0,0,0|Ln:3");
    assert(r.tick() && box.selection == 2);            // Over the line that wasn't sent
    r.report("Run|WPos:0,0,0|Ln:99");                  // Not a line of this program
    assert(r.tick() && box.selection == 2);
    r.report("Run|WPos:0,0,0|Ln:2");                   // Never back
    assert(!r.tick() && r.sender.finished && r.sender.run && box.selection == 2);
    r.report("Run|WPos:0,0,0");                        // A report without the number changes nothing
    assert(!r.tick() && box.selection == 2);
    r.report("Run|WPos:0,0,0|Ln:6");
    assert(!r.tick() && r.sender.run && box.selection == 5);
    r.report("Idle|WPos:0,0,0|Ln:6");
    assert(!r.tick() && !r.sender.run && box.Selected() == "G1 X3 (last)");
    assert((r.sent == std::vector<std::string>{"N1G21\r", "N3G1 X1 F100\r", "N4G1 X2\r", "$G\r", "N6G1 X3 (last)\r"}));
  }

  // Program in memory: selection goes to the executed line at once, there is
  // nothing to read
  {
    std::string program;
    for(int i = 1; i <= 20; i++) program += "G1 X" + std::to_string(i) + "\n";
    Rig r(program.c_str());
    for(int i = 0; i < 16; i++) assert(r.tick());
    r.report("Run|WPos:0,0,0|Ln:15");
    assert(r.tick() && r.sender.text_box.selection == 14);
  }

  // Controller reports the number only while something moves: selection waits
  {
    Rig r("G21\nG90\nG1 X1\nG1 X2");
    auto& box = r.sender.text_box;
    assert(r.tick());
    r.report("Idle|WPos:0,0,0");
    assert(r.tick() && r.tick() && box.selection == 0);
    r.report("Run|WPos:0,0,0|Ln:3");
    assert(r.tick() && r.sender.line_mode == ProgramSender::LINE_EXECUTED && box.selection == 2);
    // The last line has no motion and is never reported: it is selected when program has ended
    r.report("Run|WPos:0,0,0|Ln:4");
    assert(!r.tick() && r.sender.finished && box.selection == 3);
  }
  {
    Rig r("G1 X1\nG1 X2\nM5\nM30");
    auto& box = r.sender.text_box;
    assert(r.tick() && r.tick());
    r.report("Run|WPos:0,0,0|Ln:2");
    assert(r.tick() && r.tick() && !r.tick() && r.sender.finished && box.selection == 1);
    r.report("Idle|WPos:0,0,0");
    assert(!r.tick() && !r.sender.run && box.Selected() == "M30");
  }

  // Controller doesn't report line numbers: selection is the line to send next, as before
  {
    Rig r("G0 X1\nG0 X2\nG0 X3\nM2");
    r.sender.line_mode = ProgramSender::LINE_SENT;
    auto& box = r.sender.text_box;
    assert(r.tick() && box.selection == 1);
    box.selector_fill = 2;                             // As Run sets it for this mode
    r.report("Run|WPos:0,0,0|Ln:1");                   // Whatever it says
    assert(r.tick() && box.selection == 2 && r.tick() && box.selection == 3);
    assert(r.tick() && box.selection == 3 && !r.tick() && r.sender.finished);
    r.report("Idle|WPos:0,0,0");
    assert(!r.tick() && !r.sender.run && box.selector_fill == 0); // Program has ended: everything is executed
    assert((r.sent == std::vector<std::string>{"N1G0 X1\r", "N2G0 X2\r", "N3G0 X3\r", "N4M2\r"}));
  }

  // Number left by a program that ran before isn't taken for this one, even
  // when this one has got that far
  {
    Rig r("G0 X1\nG0 X2\nG0 X3\nG0 X4\nG0 X5");
    auto& box = r.sender.text_box;
    r.report("Idle|WPos:0,0,0|Ln:3");
    assert(r.tick() && r.tick() && r.tick() && r.tick() && r.sender.send_line == 4 && box.selection == 0);
    r.report("Run|WPos:0,0,0|Ln:1");
    assert(r.tick() && box.selection == 0 && r.sender.exec_line == 1);
    r.report("Run|WPos:0,0,0|Ln:3");
    assert(!r.tick() && box.selection == 2);
  }
  // Nor before the first line of this one is acknowledged
  {
    Rig r("; a\n; b\nG0 X1\nG0 X2");
    r.report("Idle|WPos:0,0,0|Ln:2");
    assert(r.tick() && r.sender.send_line == 3 && r.sender.text_box.selection == 0);
  }

  // The only numbered line is the last one: no command is sent after it
  {
    Rig r("$H\n$G\nG0 X1\n");
    auto& box = r.sender.text_box;
    assert(r.tick() && r.tick() && r.tick() && !r.tick() && r.sender.finished && box.selection == 0);
    r.report("Idle|WPos:0,0,0|Ln:3");
    assert(!r.tick() && !r.sender.run && r.sender.line_mode == ProgramSender::LINE_EXECUTED && box.Selected() == "G0 X1");
  }

  // Streamed program: text box holds only the visible lines and is fed from
  // the second position in the file, which follows the executed line
  const std::string file = "N1 G21\nG0 X0\n\n; note\nG1 X1\nG1 X2\nN70 G1 X3\nG1 X4\nG1 X5\nG1 X6\nG1 X7\nM2\n";
  const std::vector<std::string> shown = {"G21", "G0 X0", "", "; note", "G1 X1", "G1 X2", "G1 X3", "G1 X4", "G1 X5", "G1 X6", "G1 X7", "M2"};
  const std::vector<std::string> commands = {"N1G21\r", "N2G0 X0\r", "N5G1 X1\r", "N6G1 X2\r", "N7G1 X3\r", "N8G1 X4\r", "N9G1 X5\r",
                                             "N10G1 X6\r", "N11G1 X7\r", "N12M2\r"};
  auto open = [&](Rig& r, File& disp) {
    r.sender.text_box.SetText(nullptr);
    r.sender.p_send = nullptr;
    r.sender.SDFile.data = disp.data = file;
    r.sender.p_disp_file = &disp;
    // As the open handler does: the first screen of lines
    for(int i = 0; i < r.sender.text_box.GetNumberOfVisibleLines(); i++) {
      char str[128];
      assert(f_gets(str, sizeof(str), &disp));
      ProgramSender::StripLineNumbers(str);
      r.sender.text_box.AddLine(str);
    }
  };
  // Run the program with controller executing the line sent three commands ago
  auto run = [&](Rig& r) {
    for(int guard = 0; r.sender.run && guard < 100; guard++) {
      // Tick takes the report of the previous round, then sends
      bool sent = r.tick();
      // What is selected is the line controller executes
      if(r.sender.line_mode == ProgramSender::LINE_EXECUTED && r.sender.exec_line != 0 && !r.sender.disp_end)
        assert(r.sender.shown_line == r.sender.exec_line && r.sender.text_box.Selected() == shown[r.sender.exec_line - 1]);
      uint32_t executed = (r.sent.size() > 3) ? strtoul(r.sent[r.sent.size() - 3].c_str() + 1, nullptr, 10) : 1;
      if(!sent) executed = 12;
      r.report(std::string(sent ? "Run" : "Idle") + "|WPos:0,0,0|Ln:" + std::to_string(executed));
    }
    assert(!r.sender.run);
  };
  {
    Rig r(nullptr); File disp; open(r, disp);
    run(r);
    assert(r.sent == commands && r.sender.exec_line == 12 && r.sender.text_box.Selected() == "M2" && r.sender.msg_box.shown == 0);
  }
  // SD card fails on the position that feeds the text box: it is only the
  // picture, program goes on to the end
  {
    Rig r(nullptr); File disp; open(r, disp);
    disp.fail_after = 3;
    run(r);
    assert(r.sent == commands && r.sender.disp_end && r.sender.msg_box.shown == 0);
  }
  // It fails on the position lines are sent from: program is stopped and operator is told
  {
    Rig r(nullptr); File disp; open(r, disp);
    r.sender.SDFile.fail_after = 6;
    run(r);
    assert(r.sent.size() == 4 && r.sent.back() == "N6G1 X2\r" && r.sender.finished && r.sender.msg_box.shown == 1);
    assert(r.sender.text_box.Selected() == shown[r.sender.exec_line - 1]);
  }
  // It fails on the very first line
  {
    Rig r(nullptr); File disp; open(r, disp);
    r.sender.SDFile.fail_after = 0;
    assert(!r.tick() && r.sender.finished && r.sender.msg_box.shown == 1);
    assert(r.sender.text_box.selection == 0 && r.sender.text_box.Selected() == shown[0]);
  }
  std::cout << "Program line number and executed line tests passed\n";
}

void interpreter_tests() {
  LittleC interpreter;
  auto run = [&](const std::string& source, bool expected, const std::string& output) {
    std::vector<char> program(source.begin(), source.end()); program.push_back(0);
    std::vector<char> result(4096);
    interpreter.SetPgmBuffer(program.data(), program.size());
    interpreter.SetOutputBuf(result.data(), result.size());
    assert(interpreter.Prescan());
    interpreter.SetOutputBuf(result.data(), result.size());
    bool success = interpreter.Execute();
    if(success != expected || (expected ? std::string(result.data()) != output : std::string(result.data()).find(output) == std::string::npos)) {
      std::cerr << "Script failed: " << source << "\nActual: " << success << " " << result.data() << '\n';
      std::abort();
    }
  };
  run("int f(){switch(1){case 1:return 7;print(99);break;}return 9;}main(){print(f());}", true, "7");
  run("main(){switch(1){case 1:{print(1);break;print(9);}print(8);}print(2);}", true, "12");
  run("main(){switch(1){case 1:print(1);case 2:print(2);break;default:print(9);}print(3);}", true, "123");
  run("main(){switch(3){case 1:print(9);break;default:print(1);}print(2);}", true, "12");
  run("main(){for(int i=0;i<3;i++){switch(i){case 1:continue;default:print(i);}print(9);}}", true, "0929");
  run("main(){switch(1){case 1:print(1/0);print(9);break;}}", false, "Division by zero");
  run("main(){while(1){}}", false, "Script execution limit exceeded");
  run("main(){for(;;){}}", false, "Script execution limit exceeded");
  run("main(){do {} while(1);}", false, "Script execution limit exceeded");
  run("main(){while(1){switch(1){case 1:continue;}}}", false, "Script execution limit exceeded");
  run("main(){print(42);}", true, "42"); // A failed run must not poison the next one.
  run("int f(){return f();}main(){f();}", false, "Nesting is too deep");
  {
    std::string source = "int loop=0; int f(){while(loop){} return 1;} int g=f(); main(){print(g);}";
    std::vector<char> program(source.begin(), source.end()); program.push_back(0);
    char output[4096] = {};
    interpreter.SetPgmBuffer(program.data(), program.size());
    interpreter.SetOutputBuf(output, sizeof(output));
    assert(interpreter.Prescan());
    assert(interpreter.SetGlobalVariableValue(0, 1));
    assert(!interpreter.ResetGlobalVariableValue(1));
    assert(std::string(output).find("Script execution limit exceeded") != std::string::npos);
    assert(interpreter.SetGlobalVariableValue(0, 0));
    assert(interpreter.ResetGlobalVariableValue(1));
    // A runaway global initializer must also stop during prescan.
    program[9] = '1'; // Change the source initializer loop=0 to loop=1.
    interpreter.SetPgmBuffer(program.data(), program.size());
    interpreter.SetOutputBuf(output, sizeof(output));
    assert(!interpreter.Prescan());
    assert(std::string(output).find("Script execution limit exceeded") != std::string::npos);
  }
  std::cout << "Interpreter control-flow and execution-budget tests passed\n";
}
// What GrblComm relies on in Decimal32: FromString() tells how many characters
// the number took, ToFixedPoint() gives exact counts.
void decimal_tests() {
  Decimal32 d(7);
  assert(d.FromString("12.700,5") == 6 && d.ToFixedPoint(1000) == 12700 && d.GetExp() == 3);
  assert(d.FromString("-1.25:1]") == 5 && d.ToFixedPoint(1000) == -1250 && d.ToFixedPoint(100) == -125);
  assert(d.FromString("  +3") == 4 && d.ToFixedPoint(10000) == 30000);
  assert(d.FromString("5.") == 2 && d.FromString(".5") == 2 && d.ToFixedPoint(1000) == 500);
  assert(d.FromString("0") == 1 && d == Decimal32(0));
  // No number: nothing taken, value stays
  d = Decimal32(7);
  for(const char* bad : {"", "-", ".", "nan", "inf", ",1", "x1"}) assert(d.FromString(bad) == 0 && d == Decimal32(7));
  assert(d.FromString(nullptr) == 0);
  // Number ends at the first character that can't belong to it
  assert(d.FromString("1e99") == 1 && d.FromString("1.2.3") == 3 && d.FromString("1-2") == 1);
  // 4.035 is the number a float turns into 4034
  assert(d.FromString("4.035") == 5 && d.ToFixedPoint(1000) == 4035);
  assert(d.FromString("2.5001") == 6 && d.ToFixedPoint(10000) == 25001);
  // Extra decimals are cut, not rounded, toward zero
  assert(d.FromString("1.2349") && d.ToFixedPoint(1000) == 1234 && d.ToFixedPoint(1) == 1);
  assert(d.FromString("-1.2349") && d.ToFixedPoint(1000) == -1234 && d.ToFixedPoint(1) == -1);
  assert(d.FromString("499.6") && d.ToFixedPoint(1) == 499);
  // Out of range result is clamped
  assert(d.FromString("268435455") && d.ToFixedPoint(1000) == INT32_MAX);
  assert(d.FromString("-268435455") && d.ToFixedPoint(1000) == INT32_MIN);
  // Machine position minus work offset is exact
  Decimal32 mpos, wco;
  mpos.FromString("113.200"); wco.FromString("100.500");
  assert((mpos - wco).ToFixedPoint(1000) == 12700);
  std::cout << "Decimal32 parsing and fixed point tests passed\n";
}

int main() { comm_tests(); status_after_cmd_tests(); probe_tests(); decimal_tests(); sender_tests(); line_number_tests(); interpreter_tests(); }
