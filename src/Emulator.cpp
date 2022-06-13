#include "../inc/Emulator.hpp"

#include <bitset>
#include <fstream>

Emulator::Emulator(string filename)
  : memory(0x10000, 0) {
  ifstream rf(filename, ios::out | ios::binary);
  if(!rf) {
    error = true;
    cout << "File not found" << endl;
    return;
  }
  string byte;
  int i = 0;
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
      memory[i++] = value;
    }
  }
}

void Emulator::finished_emulation_print() {
  cout << "------------------------------------------------" << endl;
  cout << "Emulated processor executed halt instruction" << endl;
  cout << "Emulated processor state: psw=0b";
  cout << setw(16) << setfill('0') << bitset<16>(memory[PSW] << 8 + memory[PSW + 1]) << setw(0) << endl;
  cout << "r0=0x" << setw(4) << setfill('0') << hex << (memory[R0] << 8 + memory[R0 + 1]) << setw(0);
  cout << "    r1=0x" << setw(4) << setfill('0') << hex << (memory[R1] << 8 + memory[R1 + 1]) << setw(0);
  cout << "    r2=0x" << setw(4) << setfill('0') << hex << (memory[R2] << 8 + memory[R2 + 1]) << setw(0);
  cout << "    r3=0x" << setw(4) << setfill('0') << hex << (memory[R3] << 8 + memory[R3 + 1]) << setw(0) << endl;
  cout << "r4=0x" << setw(4) << setfill('0') << hex << (memory[R4] << 8 + memory[R4 + 1]) << setw(0);
  cout << "    r5=0x" << setw(4) << setfill('0') << hex << (memory[R5] << 8 + memory[R5 + 1]) << setw(0);
  cout << "    r6=0x" << setw(4) << setfill('0') << hex << (memory[R6] << 8 + memory[R6 + 1]) << setw(0);
  cout << "    r7=0x" << setw(4) << setfill('0') << hex << (memory[R7] << 8 + memory[R7 + 1]) << setw(0) << endl;
}

void Emulator::initialize() {
    memory[PC] = memory[0x0000];
    memory[PC + 1] = memory[0x0001];
}

bool Emulator::one_address_instruction() {
    return instruction == HALT || instruction == IRET || instruction == RET;
}

bool Emulator::two_address_instruction() {
    return instruction == INT || instruction == XCHG
        || instruction == ADD || instruction == SUB || instruction == MUL || instruction == DIV || instruction == CMP
        || instruction == NOT || instruction == AND || instruction == OR || instruction == XOR || instruction == TEST
        || instruction == SHL || instruction == SHR;
}

bool Emulator::three_address_instruction() {
    return (instruction == CALL || instruction == JMP || instruction == JEQ || instruction == JNE || instruction == JGT
        || instruction == LDR || instruction == STR)
        && (addressing == REGDIR || addressing == REGIND);
}

bool Emulator::wrong_op_code(int op_code) {
    return instruction != HALT && instruction != INT && instruction != IRET && instruction != CALL && instruction != RET
        && instruction != JMP && instruction != JEQ && instruction != JNE && instruction != JGT
        && instruction != XCHG
        && instruction != ADD && instruction != SUB && instruction != MUL && instruction != DIV && instruction != CMP
        && instruction != NOT && instruction != AND && instruction != OR && instruction != XOR && instruction != TEST
        && instruction != SHL && instruction != SHR && instruction != LDR && instruction != STR;
}

bool Emulator::wrong_addressing(int addressing) {
    return addressing != IMMED
        && addressing != REGDIR && addressing != REGDIRADD
        && addressing != REGIND && addressing != REGINDPOM
        && addressing != MEM;
}

bool Emulator::wrong_update(int update) {
    return update < 0 || update > 4;
}

bool Emulator::wrong_register(int reg) {
    return reg != 15 && (reg > 8 || reg < 0);
}

void Emulator::fetch() {
    error = false;
    int ins = memory[memory[PC]++];
    if(wrong_op_code(ins)) {
        error = true;
        return;
    }
    instruction = (Instruction) ins;
    if(one_address_instruction()) return;

    int registers = memory[memory[PC]++];
    if(wrong_register(registers >> 4) || wrong_register(registers & 0xf)) {
        error = true;
        return;
    }
    reg_D = (Register) (0xff20 + (registers >> 4));
    reg_S = (Register) (0xff20 + (registers & 0xf));
    if(two_address_instruction()) return;

    int address_update = memory[memory[PC]++];
    if(wrong_addressing(address_update & 0xf) || wrong_update(address_update >> 4)) {
        error = true;
        return;
    }
    addressing = (Addressing) (address_update & 0xf);
    update = (Register_Update) (address_update >> 4);
    if(three_address_instruction()) return;

    int data_high = memory[memory[PC]++];
    int data_low = memory[memory[PC]++];
    data_payload = (data_high << 8) + data_low;
}

void Emulator::addr() {
    if(addressing == IMMED) {
        operand = data_payload;
    }
    else if(addressing == REGDIR) {
        operand = (memory[reg_S] << 8) + memory[reg_S + 1];
    }
    else if(addressing == REGDIRADD) {
        operand = (memory[reg_S] << 8) + memory[reg_S + 1] + data_payload;
    }
    else if(addressing == REGIND || addressing == REGINDPOM) {
        if(update == PREDECR || update == PREINCR) {
            int reg_s = (memory[reg_S] << 8) + memory[reg_S + 1] + (update == PREDECR ? -2 : 2);
            memory[reg_S] = reg_s >> 8;
            memory[reg_S + 1] = reg_s & 0xff;
        }
        int addr = (memory[reg_S] << 8) + memory[reg_S + 1] + (addressing == REGINDPOM ? data_payload : 0);
        operand = (memory[addr] << 8) + memory[addr + 1];
        if(instruction == STR) {
            operand = addr;
        }
        if(update == POSTDECR || update == POSTINCR) {
            int reg_s = (memory[reg_S] << 8) + memory[reg_S + 1] + (update == POSTDECR ? -2 : 2);
            memory[reg_S] = reg_s >> 8;
            memory[reg_S + 1] = reg_s & 0xff;
        }
    }
    else if(addressing == MEM) {
        operand = (memory[data_payload] << 8) + memory[data_payload + 1];
        if(instruction == STR) {
            operand = data_payload;
        }
    }
}

void Emulator::exec() {
    if(instruction == HALT) {
        emulation_over = true;
    }
    else if(instruction == INT) {
        int sp = (memory[SP] << 8) + memory[SP + 1] - 4;
        memory[sp] = memory[PC];
        memory[sp + 1] = memory[PC + 1];
        memory[sp + 2] = memory[PSW];
        memory[sp + 3] = memory[PSW + 1];
        memory[SP] = sp >> 8;
        memory[SP + 1] = sp & 0xff;
        int reg_d = (memory[reg_D] << 8) + memory[reg_D + 1];
        int new_addr = (memory[reg_d + 1] & 7) << 1;
        memory[PC] = memory[new_addr];
        memory[PC + 1] = memory[new_addr + 1];
    }
    else if(instruction == IRET) {
        int sp = (memory[SP] << 8) + memory[SP + 1] + 4;
        memory[PSW + 1] = memory[sp - 4];
        memory[PSW] = memory[sp - 3];
        memory[PC + 1] = memory[sp - 2];
        memory[PC] = memory[sp - 1];
        memory[SP] = sp >> 8;
        memory[SP + 1] = sp & 0xff;
    }
    else if(instruction == CALL) {
        int sp = (memory[SP] << 8) + memory[SP + 1] - 2;
        memory[sp] = memory[PC];
        memory[sp + 1] = memory[PC + 1];
        memory[SP] = sp >> 8;
        memory[SP + 1] = sp & 0xff;
        memory[PC] = operand >> 8;
        memory[PC + 1] = operand & 0xff;
    }
    else if(instruction == RET) {
        int sp = (memory[SP] << 8) + memory[SP + 1] + 2;
        memory[PC + 1] = memory[sp - 2];
        memory[PC] = memory[sp - 1];
        memory[SP] = sp >> 8;
        memory[SP + 1] = sp & 0xff;
    }
    else if(instruction == JMP || instruction == JEQ || instruction == JNE || instruction == JGT) {
        bool N = memory[PSW + 1] & 8;
        bool O = memory[PSW + 1] & 2;
        bool Z = memory[PSW + 1] & 1;
        if(instruction == JMP
        || instruction == JEQ && Z
        || instruction == JNE && !Z
        || instruction == JGT && !Z && (!N && !O || N && O)) {
            memory[PC] = operand >> 8;
            memory[PC + 1] = operand & 0xff;
        }
    }
    else if(instruction == XCHG) {
        int hi = memory[reg_D], lo = memory[reg_D + 1];
        memory[reg_D] = memory[reg_S], memory[reg_D + 1] = memory[reg_S + 1];
        memory[reg_S] = hi, memory[reg_S + 1] = lo;
    }
    else if(instruction == ADD || instruction == SUB || instruction == MUL || instruction == DIV || instruction == CMP
        || instruction == NOT || instruction == AND || instruction == OR || instruction == XOR || instruction == TEST
        || instruction == SHL || instruction == SHR) {
        int reg_d = (memory[reg_D] << 8) + memory[reg_D + 1];
        int reg_s;
        if(instruction != NOT) reg_s = (memory[reg_S] << 8) + memory[reg_S + 1];
        int res;
        if(instruction == ADD) res = reg_d + reg_s;
        else if(instruction == SUB || instruction == CMP) res = reg_d - reg_s;
        else if(instruction == MUL) res = reg_d * reg_s;
        else if(instruction == DIV) res = reg_d / reg_s;
        else if(instruction == NOT) res = ~reg_d;
        else if(instruction == AND || instruction == TEST) res = reg_d & reg_s;
        else if(instruction == OR) res = reg_d | reg_s;
        else if(instruction == XOR) res = reg_d ^ reg_s;
        else if(instruction == SHL) res = reg_d << reg_s;
        else if(instruction == SHR) res = reg_d >> reg_s;

        int psw = (memory[PSW] << 8) + memory[PSW + 1];
        if(res == 0) psw |= 0x1;
        if(res < 0) psw |= 0x8;
        if(instruction == SHL && (res >> 15)
        || (instruction == ADD || instruction == MUL) && res > 0xffff
        || (instruction == CMP || instruction == SUB) && (reg_d < reg_s)) psw |= 0x4;
        res &= 0xff;
        if(instruction != TEST && instruction != CMP) {
            memory[reg_D] = res >> 8;
            memory[reg_D + 1] = res & 0xff;
        }
        memory[PSW] = psw >> 8;
        memory[PSW + 1] = psw & 0xff;
    }
    else if(instruction == LDR) {
        memory[reg_D] = operand >> 8;
        memory[reg_D + 1] = operand & 0xff;
    }
    else if(instruction == STR) {
        memory[operand] = memory[reg_D];
        memory[operand + 1] = memory[reg_D + 1];
    }
}

void Emulator::intr() {

}

void Emulator::emulate() {
    initialize();
}
