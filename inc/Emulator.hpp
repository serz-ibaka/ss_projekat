#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

using namespace std;

class Emulator {
  typedef enum { R0, R1, R2, R3, R4, R5, R6, R7, PSW, SP = 6, PC = 7 } Register;

  vector<int> registers;
  vector<int> memory;
};

#endif
