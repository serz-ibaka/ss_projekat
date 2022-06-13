#include "../inc/Emulator.hpp"

#include <bitset>
#include <fstream>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>

void Emulator::getch() {
    char buf = 0;
    if(read(0, &buf, 1) < 0) {
        // perror("read()");
        return;
    }
    memory[TERM_IN] = +buf;
    memory[PSW] &= ~0x40;
}

void Emulator::update_terminal() {
    getch();
}

void Emulator::update_timer() {
    int timer_index = (memory[TIM_CFG] << 8) + memory[TIM_CFG + 1];
    int timer = 500;
    if(timer_index < timer_values.size()) timer = timer_values[timer_index];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    if(ms - last_interrupt > timer) {
        last_interrupt = ms;
        memory[PSW] &= ~0x20;
    }
}

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
  cout << endl << endl << "------------------------------------------------" << endl;
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
    memory[SP] = 0xff;
    memory[SP + 1] = 0x00;
    memory[PSW] |= 0x60;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    last_interrupt = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    old = {0};
    if(tcgetattr(0, &old) < 0) {
        perror("tcsetattr()");
    }
    old.c_lflag &= ~(ICANON | ECHO);
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0) {
        perror("tcsetattr ICANON");
    }

    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);

    // memory[TIM_CFG + 1] = 0x7;
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
    return op_code != HALT && op_code != INT && op_code != IRET && op_code != CALL && op_code != RET
        && op_code != JMP && op_code != JEQ && op_code != JNE && op_code != JGT
        && op_code != XCHG
        && op_code != ADD && op_code != SUB && op_code != MUL && op_code != DIV && op_code != CMP
        && op_code != NOT && op_code != AND && op_code != OR && op_code != XOR && op_code != TEST
        && op_code != SHL && op_code != SHR && op_code != LDR && op_code != STR;
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
    int pc = (memory[PC] << 8) + memory[PC + 1];
    int ins = memory[pc++];
    memory[PC] = pc >> 8;
    memory[PC + 1] = pc & 0xff;
    if(wrong_op_code(ins)) {
        error = true;
        return;
    }
    instruction = (Instruction) ins;
    if(one_address_instruction()) return;

    int registers = memory[pc++];
    memory[PC] = pc >> 8;
    memory[PC + 1] = pc & 0xff;
    if(wrong_register(registers >> 4) || wrong_register(registers & 0xf)) {
        error = true;
        return;
    }
    reg_D = (Register) (0xff20 + (registers >> 4));
    reg_S = (Register) (0xff20 + (registers & 0xf));
    if(two_address_instruction()) return;

    int address_update = memory[pc++];
    memory[PC] = pc >> 8;
    memory[PC + 1] = pc & 0xff;
    if(wrong_addressing(address_update & 0xf) || wrong_update(address_update >> 4)) {
        error = true;
        return;
    }
    addressing = (Addressing) (address_update & 0xf);
    update = (Register_Update) (address_update >> 4);
    if(three_address_instruction()) return;

    int data_high = memory[pc++];
    int data_low = memory[pc++];
    memory[PC] = pc >> 8;
    memory[PC + 1] = pc & 0xff;
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
        if(operand == TERM_OUT) {
            cout << (char)memory[TERM_OUT + 1];
        }
    }
}

void Emulator::intr() {
    bool timer_interrupt = !(memory[PSW] & 0x20);
    bool terminal_interrupt = !(memory[PSW] & 0x40);
    if(error || timer_interrupt || terminal_interrupt) {
        int sp = (memory[SP] << 8) + memory[SP + 1] - 4;
        memory[sp] = memory[PC];
        memory[sp + 1] = memory[PC + 1];
        memory[sp + 2] = memory[PSW];
        memory[sp + 3] = memory[PSW + 1];
        memory[SP] = sp >> 8;
        memory[SP + 1] = sp & 0xff;
    
        if(error) {
            memory[PC] = memory[0x2];
            memory[PC + 1] = memory[0x3];
        }
        else if(!(memory[PSW] & 0x40)) {
            memory[PC] = memory[0x6];
            memory[PC + 1] = memory[0x7];
            memory[PSW] |= 0x40;
        }
        else if(!(memory[PSW] & 0x20)) {
            memory[PC] = memory[0x4];
            memory[PC + 1] = memory[0x5];
            memory[PSW] |= 0x20;
        }
    }
}

void Emulator::emulate() {
    initialize();
    while(!emulation_over) {
        fetch();
        addr();
        // cout << instruction << " " << addressing << endl;
        exec();
        update_terminal();
        update_timer();
        intr();
    }


    old.c_lflag |= ICANON | ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0) {
        perror("tcsetattr ~ICANON");
    }
}
