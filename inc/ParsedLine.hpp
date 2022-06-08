#ifndef _PARSEDLINE_H
#define _PARSEDLINE_H

#include <string>
#include <vector>
#include <utility>

using namespace std;

class Parser;
class Assembler;

class ParsedLine {
    typedef enum { NONE_TYPE, DIRECTIVE, INSTRUCTION, LABEL } Type;
    typedef enum { NONE_REG, R0, R1, R2, R3, R4, R5, SP, PC, PSW } Register;
    typedef enum { NONE_ADDR, LIT, SYM, PCREL, MEM_LIT, MEM_SYM, REG_DIR, REG_IND, REG_IND_LIT, REG_IND_SYM } Addressing;
    typedef enum { NONE_INS, HALT, INT, IRET, CALL, RET, JMP, JEQ, JNE, JGT, PUSH, POP,
                   XCHG, ADD, SUB, MUL, DIV, CMP, NOT, AND, OR, XOR, TEST, SHL, SHR, LDR, STR } Instruction;
    typedef enum { NONE_DIR, GLOBAL, EXTERN, SECTION, WORD, SKIP, ASCII, EQU, END } Directive;

    Instruction instruction = NONE_INS;
    Directive directive = NONE_DIR;
    Type type = NONE_TYPE;
    Register reg_dst = NONE_REG;            // used when one register as parameter
    Register reg_src = NONE_REG;
    Addressing addressing = NONE_ADDR;
    string operand_symbol;                  // symbol
    int operand_literal = -1;               // literal
    vector<string> labels;
    vector<string> symbols;                 // symbols and literals for .word,
                                            // symbols for .extern, .global
    vector<pair<string, bool>> expression;  // expression for .equ
    string ascii_string;                    // param for .ascii

    int param_count = 0;

    bool error = false;
    bool empty = false;

    bool jump_instruction();
    bool address_instruction();

    static void strip(string& str);
    static void clean_comment(string& str);

    void set_command(string& command);
    static Register get_register(string& str);
    void add_param(string param);

    static bool check_symbol(string& str);
    static bool check_literal(string& str);
    static bool check_expression(string& str);
    static int convert_literal(string& str);
    void set_expression(string& str);

    void final_check();

public:

    ParsedLine(string line);

    friend class Parser;
    friend class Assembler;

};


#endif
