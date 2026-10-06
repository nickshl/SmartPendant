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
    assert(comm.grbl_probe_position[0] == 1 && comm.grbl_probe_position[1] == 2 && comm.grbl_probe_position[2] == 3);
  }
  parse("[PRB:-1.25,0,3.5:0]");
  assert(comm.IsProbeDataReceived() && !comm.IsProbeSucceed());
  assert(comm.grbl_probe_position[0] == -1.25f);
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
      assert(sender.run && comm.messages.size() == 1 && sender.text_box.selection == 1);
    } else {
      assert(comm.messages.empty() && sender.text_box.selection == 0);
      assert(sender.run == (status == GrblComm::Status_Cmd_Not_Executed_Yet));
    }
  }
  std::cout << "Program streaming acknowledgement tests passed\n";
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
int main() { comm_tests(); probe_tests(); sender_tests(); interpreter_tests(); }
