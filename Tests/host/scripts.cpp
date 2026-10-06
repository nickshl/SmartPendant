#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include "GrblComm.h"
#include "Little-C.h"
int main(int argc, char** argv) {
  if(argc != 2) return 2;
  std::ifstream input(argv[1]);
  if(!input) return 2;
  std::vector<char> source((std::istreambuf_iterator<char>(input)), {});
  source.push_back('\0');
  std::vector<char> output(1024 * 1024);
  auto& comm = GrblComm::GetInstance();
  comm.number_of_axis = 3;
  comm.grbl_position[0] = 10.0f;
  comm.grbl_position[1] = 20.0f;
  comm.grbl_position[2] = 30.0f;
  LittleC interpreter;
  interpreter.SetPgmBuffer(source.data(), source.size());
  interpreter.SetOutputBuf(output.data(), output.size());
  if(!interpreter.Prescan()) { std::cerr << output.data(); return 1; }
  interpreter.SetOutputBuf(output.data(), output.size());
  if(!interpreter.Execute()) { std::cerr << output.data(); return 1; }
  std::cout << output.data();
}
