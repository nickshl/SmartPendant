// Runs one script from Scripts/ the way the pendant does and prints either the
// generated G-code or the parameter menu. Everything the script touches is the
// real firmware code: the interpreter, GrblComm(positions arrive as a status
// report and go through the real parser) and the menu comment parser of
// GCodeGeneratorScr. Used by script_checks.py, see README.md.
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include "menu.h" // GrblComm.h and Little-C.h come with it; Little-C.h has no include guard

// Controller side of the UART: lines queued here are read by GrblComm::PollSerial()
struct ControllerUart : IUart {
  std::deque<uint8_t> incoming;
  Result Init() override { return Result::RESULT_OK; }
  Result SetBaudRate(uint32_t) override { return Result::RESULT_OK; }
  Result Read(uint8_t& byte) override {
    if(incoming.empty()) return Result::ERR_UART_EMPTY;
    byte = incoming.front(); incoming.pop_front(); return Result::RESULT_OK;
  }
  Result Write(uint8_t*, uint32_t) override { return Result::RESULT_OK; }
  void line(const std::string& text) { incoming.insert(incoming.end(), text.begin(), text.end()); incoming.push_back('\n'); }
};

static int usage() {
  std::cerr <<
    "usage: script_runner <script> [options] [name=value ...]\n"
    "  -x V -y V -z V  work position exactly as the controller prints it: mm, or\n"
    "                  inches with -i; X is a diameter with -d (default 10 20 30)\n"
    "  -i              controller reports inches($13=1)\n"
    "  -d              lathe diameter mode(D:1 in the status report)\n"
    "  -w X,Y,Z        report machine position(MPos) plus this work offset(WCO)\n"
    "                  instead of WPos; the work position stays what -x -y -z say\n"
    "  -m              print the parameter menu instead of the program\n"
    "  -g N            press Generate N times first(default 1, with -m 0). The pendant\n"
    "                  doesn't reload the script in between, and neither does this\n"
    "  -b N            size of the output buffer in bytes(default 1048576)\n"
    "  name=value      set a parameter(global variable) to a raw integer value\n"
    "exit code: 0 - program or menu is on stdout, 3 - script error, its text is on\n"
    "stderr, 2 - bad arguments. Anything else is a crash(sanitizers exit with 1).\n";
  return 2;
}

// Position as the controller prints it: optional minus, digits, optional fraction
static bool is_position(const std::string& text) {
  size_t i = (text[0] == '-') ? 1 : 0, digits = 0;
  for(; i < text.size(); i++) {
    if(isdigit((unsigned char)text[i])) digits++;
    else if(text[i] != '.' || text.find('.') != i) return false;
  }
  return digits != 0;
}

int main(int argc, char** argv) {
  if(argc < 2) return usage();
  std::string pos[3] = {"10.000", "20.000", "30.000"}, wco;
  bool inch = false, diameter = false, menu = false;
  int generate = -1;
  size_t out_size = 1024 * 1024;
  std::vector<std::pair<std::string, int>> sets;
  for(int i = 2; i < argc; i++) {
    std::string a = argv[i];
    auto value = [&]() -> std::string { if(i + 1 >= argc) exit(usage()); return argv[++i]; };
    if(a == "-x" || a == "-y" || a == "-z") {
      std::string v = value();
      if(!is_position(v)) { std::cerr << "not a position: " << v << "\n"; return 2; }
      pos[a[1] - 'x'] = v;
    }
    else if(a == "-w") wco = value();
    else if(a == "-b") out_size = strtoul(value().c_str(), nullptr, 10);
    else if(a == "-g") generate = atoi(value().c_str());
    else if(a == "-i") inch = true;
    else if(a == "-d") diameter = true;
    else if(a == "-m") menu = true;
    else if(a.find('=') != std::string::npos && a[0] != '-') {
      char* end = nullptr;
      const char* number = a.c_str() + a.find('=') + 1;
      long v = strtol(number, &end, 10);
      if(end == number || *end != '\0') { std::cerr << "not an integer: " << a << "\n"; return 2; }
      sets.emplace_back(a.substr(0, a.find('=')), (int)v);
    }
    else return usage();
  }

  std::ifstream input(argv[1]);
  if(!input) { std::cerr << "can't open " << argv[1] << "\n"; return 2; }
  std::vector<char> source((std::istreambuf_iterator<char>(input)), {});
  source.push_back('\0');
  std::vector<char> output(out_size);

  // Bring the communication layer to the state it has on a connected pendant
  // by sending it what the controller sends.
  auto& comm = GrblComm::GetInstance();
  ControllerUart uart;
  comm.uart = &uart;
  comm.grbl_state = GrblComm::IDLE;
  comm.grbl_mpgMode = true;
  comm.request_settings = false;
  uart.line("[AXS:3:XYZ]");
  uart.line(inch ? "$13=1" : "$13=0");
  std::string report = "<Idle|";
  if(wco.empty()) {
    report += "WPos:" + pos[0] + "," + pos[1] + "," + pos[2];
  } else {
    // Machine position = work position + offset, printed with controller precision
    double w[3] = {0, 0, 0};
    if(sscanf(wco.c_str(), "%lf,%lf,%lf", &w[0], &w[1], &w[2]) != 3) { std::cerr << "not an offset: " << wco << "\n"; return 2; }
    char buf[128];
    const char* fmt = inch ? "MPos:%.4f,%.4f,%.4f|WCO:%.4f,%.4f,%.4f" : "MPos:%.3f,%.3f,%.3f|WCO:%.3f,%.3f,%.3f";
    snprintf(buf, sizeof(buf), fmt, atof(pos[0].c_str()) + w[0], atof(pos[1].c_str()) + w[1], atof(pos[2].c_str()) + w[2], w[0], w[1], w[2]);
    report += buf;
  }
  report += std::string("|FS:0,0|D:") + (diameter ? "1" : "0") + ">";
  uart.line(report);
  comm.PollSerial();

  GCodeGeneratorScr gen;
  LittleC& interpreter = gen.interpreter;
  interpreter.SetPgmBuffer(source.data(), source.size());
  interpreter.SetOutputBuf(output.data(), output.size());
  if(!interpreter.Prescan()) { std::cerr << output.data() << "\n"; return 3; }

  int cnt = interpreter.GetGlobalVariablesCnt();
  for(auto& s : sets) {
    bool found = false;
    for(int i = 0; i < cnt; i++) {
      char name[64] = "";
      interpreter.GetGlobalVariableName(i, name, sizeof(name));
      if(s.first == name) { interpreter.SetGlobalVariableValue(i, s.second); found = true; }
    }
    if(!found) { std::cerr << "no parameter named " << s.first << "\n"; return 2; }
  }

  // Generate, as many times as asked. Like on the pendant the script isn't
  // prescanned again: what it did to its global variables stays.
  if(generate < 0) generate = menu ? 0 : 1;
  for(int n = 0; n < generate; n++) {
    interpreter.SetOutputBuf(output.data(), output.size());
    if(!interpreter.Execute()) { std::cerr << output.data() << "\n"; return 3; }
    interpreter.SetOutputBuf(nullptr, 0);
  }

  if(menu) {
    // One line per parameter, tab separated:
    //   name, label, kind(num|enum), scaler, units, min, max, value, row
    // For enum min is 0 and max is number of values - 1. Row is the menu line
    // as the pendant shows it: same buffer sizes as in UpdateMenuStrings()
    // and the 32 character line of Menu::CreateString().
    for(int i = 0; i < cnt; i++) {
      char name[64] = "", var_str[24u] = {0}, val_str[24u] = {0}, units_str[8u] = {0}, row[33] = {0};
      int var_val = 0;
      int32_t scaler = 1, min = 0, max = 0;
      interpreter.GetGlobalVariableName(i, name, sizeof(name));
      interpreter.GetGlobalVariableValue(i, var_val);
      gen.GetGlobalVariableDescription(i, var_str, NumberOf(var_str));
      bool is_enum = gen.IsGlobalVariableEnum(i);
      if(is_enum) {
        gen.GetGlobalVariableEnumValue(i, val_str, NumberOf(val_str), var_val);
        scaler = 0; max = gen.GetGlobalVariableEnumCount(i) - 1;
      } else {
        gen.GetGlobalVariableScaler(i, scaler);
        gen.GetGlobalVariableUnits(i, units_str, NumberOf(units_str));
        gen.GetGlobalVariableMinVal(i, min);
        gen.GetGlobalVariableMaxVal(i, max);
        comm.ValueToStringWithScalerAndUnits(val_str, NumberOf(val_str), var_val, scaler, units_str);
      }
      int remaining = 32 - (int)strlen(var_str);
      if(remaining > 0) snprintf(row, sizeof(row), "%s%*s", var_str, remaining, val_str);
      else              snprintf(row, sizeof(row), "%s", var_str);
      printf("%s\t%s\t%s\t%d\t%s\t%d\t%d\t%d\t%s\n", name, var_str, is_enum ? "enum" : "num", (int)scaler, units_str, (int)min, (int)max, var_val, row);
    }
    return 0;
  }

  if(generate > 0) std::cout << output.data();
  return 0;
}
