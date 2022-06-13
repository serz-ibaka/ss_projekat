#include "../inc/Emulator.hpp"

int main(int argc, char* argv[]) {
  if(argc != 2) {
    cout << "Error : wrong number of parameters for emulation" << endl;
    return 0; 
  }
  string filename = argv[1];
  Emulator emulator(filename);
  emulator.emulate();
  emulator.finished_emulation_print();
}