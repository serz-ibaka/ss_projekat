#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sys/time.h>

using namespace std;

class Emulator {
  bool error = false;
  bool emulation_over = false;

  vector<int> memory;

  typedef enum { TERM_OUT = 0xff00, TERM_IN = 0xff02, TIM_CFG = 0xff10,
                 R0 = 0xff20, R1 = 0xff22, R2 = 0xff24, R3 = 0xff26, R4 = 0xff28, R5 = 0xff2a, R6 = 0xff2c, R7 = 0xff2e,
                 PSW = 0xff30, SP = 0xff2c, PC = 0xff2e, NONE_REG = 0xff2f } Register;

  typedef enum { START, ERROR = 0x0002, TIMER = 0x0004, TERMINAL = 0x0006 } Interrupt;

  typedef enum { HALT, INT = 0x10, IRET = 0x20, CALL = 0x30, RET = 0x40,
                 JMP = 0x50, JEQ, JNE, JGT,
                 XCHG = 0x60,
                 ADD = 0x70, SUB, MUL, DIV, CMP,
                 NOT = 0x80, AND, OR, XOR, TEST,
                 SHL = 0x90, SHR,
                 LDR = 0xa0, STR = 0xb0 } Instruction;

  typedef enum { IMMED, REGDIR, REGDIRADD, REGIND, REGINDPOM, MEM } Addressing;

  typedef enum { NONE, PREDECR, PREINCR, POSTDECR, POSTINCR } Register_Update;

  const vector<int> timer_values = { 500, 1000, 1500, 2000, 5000, 10000, 30000, 60000, };

  long last_interrupt; 

  Instruction instruction;
  Register reg_D, reg_S;
  Addressing addressing;
  Register_Update update;
  int data_payload;
  int operand;

  bool one_address_instruction();
  bool two_address_instruction();
  bool three_address_instruction();
  bool wrong_op_code(int op_code);
  bool wrong_register(int reg);
  bool wrong_addressing(int addressing);
  bool wrong_update(int update);

  void getch();
  void update_terminal();
  void update_timer();

public:
  Emulator(string filename);

  void initialize();
  void fetch();
  void addr();
  void exec();
  void intr();
  void emulate();
  void finished_emulation_print();

};

#endif
