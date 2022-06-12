#include "../inc/Emulator.hpp"

#include <bitset>
#include <fstream>

Emulator::Emulator(string filename)
  : registers(8, 0) {
  ifstream rf(filename, ios::out | ios::binary);
  if(!rf) {
    error = true;
    cout << "File not found" << endl;
    return;
  }
  string byte;
  while(rf >> byte) {
    if(byte.size() == 2) {
      int value = 0;
      if(byte[0] >= '0' && byte[0] <= '9') {
        value += 16 * (byte[0] - '0');
      } else if(byte[0] >= 'a' && byte[0] <= 'f') {
        value += 16 * (10 + byte[0] - 'a');
      }
      if(byte[1] >= '0' && byte[1] <= '9') {
        value += byte[1] - '0';
      } else if(byte[1] >= 'a' && byte[1] <= 'f') {
        value += 10 + byte[1] - 'a';
      }
      memory.push_back(value);
    }
  }
}

void Emulator::finished_emulation_print() {
  cout << "------------------------------------------------" << endl;
  cout << "Emulated processor executed halt instruction" << endl;
  cout << "Emulated processor state: psw=0b";
  cout << setw(16) << setfill('0') << bitset<16>(registers[PSW]) << setw(0) << endl;
  cout << "r0=0x" << setw(4) << setfill('0') << hex << registers[R0] << setw(0);
  cout << "    r1=0x" << setw(4) << setfill('0') << hex << registers[R1] << setw(0);
  cout << "    r2=0x" << setw(4) << setfill('0') << hex << registers[R2] << setw(0);
  cout << "    r3=0x" << setw(4) << setfill('0') << hex << registers[R3] << setw(0) << endl;
  cout << "r4=0x" << setw(4) << setfill('0') << hex << registers[R4] << setw(0);
  cout << "    r5=0x" << setw(4) << setfill('0') << hex << registers[R5] << setw(0);
  cout << "    r6=0x" << setw(4) << setfill('0') << hex << registers[R6] << setw(0);
  cout << "    r7=0x" << setw(4) << setfill('0') << hex << registers[R7] << setw(0) << endl;
}