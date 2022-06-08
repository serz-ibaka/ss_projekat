#include "../inc/ParsedLine.hpp"

#include <regex>


bool ParsedLine::jump_instruction() {
    return instruction == CALL || instruction == JMP ||
        instruction == JEQ || instruction == JNE || instruction == JGT;
}

bool ParsedLine::address_instruction() {
    return instruction == LDR || instruction == STR;
}

void ParsedLine::strip(string &str) {
    int len = str.size();
    int start_index = 0, end_index = len - 1;
    while(start_index <= end_index && (str[start_index] == ' ' || str[start_index] == '\t')) start_index++;
    while(start_index <= end_index && (str[end_index] == ' ' || str[end_index] == '\t')) end_index--;
    str = str.substr(start_index, end_index - start_index + 1);
}

void ParsedLine::clean_comment(string &str) {
    int comment_index = 0, len = str.size();
    while(comment_index < len && str[comment_index] != '#') comment_index++;
    if(comment_index < len) str = str.substr(0, comment_index);
}

ParsedLine::Register ParsedLine::get_register(string &str) {
    if(str == "r0") return R0;
    else if(str == "r1") return R1;
    else if(str == "r2") return R2;
    else if(str == "r3") return R3;
    else if(str == "r4") return R4;
    else if(str == "r5") return R5;
    else if(str == "r6" || str == "sp") return SP;
    else if(str == "r7" || str == "pc") return PC;
    else if(str == "psw") return PSW;
    else return NONE_REG;
}

void ParsedLine::set_command(string& command) {
    if(command == ".global") directive = GLOBAL;
    else if(command == ".extern") directive = EXTERN;
    else if(command == ".section") directive = SECTION;
    else if(command == ".word") directive = WORD;
    else if(command == ".skip") directive = SKIP;
    else if(command == ".ascii") directive = ASCII;
    else if(command == ".equ") directive = EQU;
    else if(command == ".end") directive = END;
    else if(command == "halt") instruction = HALT;
    else if(command == "int") instruction = INT;
    else if(command == "iret") instruction = IRET;
    else if(command == "call") instruction = CALL;
    else if(command == "ret") instruction = RET;
    else if(command == "jmp") instruction = JMP;
    else if(command == "jeq") instruction = JEQ;
    else if(command == "jne") instruction = JNE;
    else if(command == "jgt") instruction = JGT;
    else if(command == "push") instruction = PUSH;
    else if(command == "pop") instruction = POP;
    else if(command == "xchg") instruction = XCHG;
    else if(command == "add") instruction = ADD;
    else if(command == "sub") instruction = SUB;
    else if(command == "mul") instruction = MUL;
    else if(command == "div") instruction = DIV;
    else if(command == "cmp") instruction = CMP;
    else if(command == "not") instruction = NOT;
    else if(command == "and") instruction = AND;
    else if(command == "or") instruction = OR;
    else if(command == "xor") instruction = XOR;
    else if(command == "test") instruction = TEST;
    else if(command == "shl") instruction = SHL;
    else if(command == "shr") instruction = SHR;
    else if(command == "ldr") instruction = LDR;
    else if(command == "str") instruction = STR;

    if(directive != NONE_DIR) type = DIRECTIVE;
    else if(instruction != NONE_INS) type = INSTRUCTION;
}

void ParsedLine::add_param(string param) {
    param_count++;
    Register reg = get_register(param);

    if(reg != NONE_REG) {
        if(reg_dst == NONE_REG) reg_dst = reg;
        else if(reg_src == NONE_REG) {
            reg_src = reg;
            if(address_instruction()) addressing = REG_DIR;
        }
        else {
            error = true;
            return;
        }
    }
    else if(jump_instruction()) {
        if(check_symbol(param)) {
            operand_symbol = param;
            addressing = SYM;
        } else if(check_literal(param)) {
            operand_literal = convert_literal(param);
            addressing = LIT;
        }
        else if(param[0] == '%') {
            string real_param = param.substr(1);
            if(check_symbol(real_param)) {
                operand_symbol = real_param;
                addressing = PCREL;
                reg_dst = PC;
            } else {
                error = true;
                return;
            }
        } else if(param[0] == '*') {
            string real_param = param.substr(1);
            if(get_register(real_param) != NONE_REG) {
                reg_dst = get_register(real_param);
                addressing = REG_DIR;
            } else if(check_literal(real_param)) {
                operand_literal = convert_literal(real_param);
                addressing = MEM_LIT;
            } else if(check_symbol(real_param)) {
                operand_symbol = real_param;
                addressing = MEM_SYM;
            } else {
                if(real_param[0] == '[' && real_param[real_param.size() - 1] == ']') {
                    real_param = real_param.substr(1, real_param.size() - 2);
                    if(get_register(real_param) != NONE_REG) {
                        reg_dst = get_register(real_param);
                        addressing = REG_IND;
                    } else {
                        int i = 0;
                        string reg, op;
                        while(i < real_param.size() && real_param[i] != '+') i++;
                        if(i >= real_param.size()) {
                            error = true;
                            return;
                        }
                        reg = real_param.substr(0, i);
                        op = real_param.substr(i + 1);
                        strip(reg); strip(op);
                        if(get_register(reg) == NONE_REG) {
                            error = true;
                            return;
                        } else {
                            reg_dst = get_register(reg);
                        }
                        if(check_symbol(op)) {
                            operand_symbol = op;
                            addressing = REG_IND_SYM;
                        } else if(check_literal(op)) {
                            operand_literal = convert_literal(op);
                            addressing = REG_IND_LIT;
                        } else {
                            error = true;
                            return;
                        }
                    }
                } else {
                    error = true;
                    return;
                }
            }
        }
    }
    else if(address_instruction()) {
        if(check_symbol(param)) {
            operand_symbol = param;
            addressing = MEM_SYM;
        } else if(check_literal(param)) {
            operand_literal = convert_literal(param);
            addressing = MEM_LIT;
        } else if(param[0] == '$') {
            string real_param = param.substr(1);
            if(check_symbol(real_param)) {
                operand_symbol = real_param;
                addressing = SYM;
            } else if(check_literal(real_param)) {
                operand_literal = convert_literal(real_param);
                addressing = LIT;
            } else {
                error = true;
                return;
            }
        } else if(param[0] == '%') {
            string real_param = param.substr(1);
            if(check_symbol(real_param)) {
                operand_symbol = real_param;
                addressing = PCREL;
                reg_dst = PC;
            } else {
                error = true;
                return;
            }
        } else if(param[0] == '[' && param[param.size() - 1] == ']') {
            string real_param = param.substr(1, param.size() - 2);
            if(get_register(real_param) != NONE_REG) {
                reg_dst = get_register(real_param);
                addressing = REG_IND;
            } else {
                int i = 0;
                string reg, op;
                while(i < real_param.size() && real_param[i] != '+') i++;
                if(i >= real_param.size()) {
                    error = true;
                    return;
                }
                reg = real_param.substr(0, i);
                op = real_param.substr(i + 1);
                strip(reg); strip(op);
                if(get_register(reg) == NONE_REG) {
                    error = true;
                    return;
                } else {
                    reg_dst = get_register(reg);
                }
                if(check_symbol(op)) {
                    operand_symbol = op;
                    addressing = REG_IND_SYM;
                } else if(check_literal(op)) {
                    operand_literal = convert_literal(op);
                    addressing = REG_IND_LIT;
                } else {
                    error = true;
                    return;
                }
            }
        } else {
            error = true;
            return;
        }
    }
    else if(directive == ASCII) {
        if(param[0] != '"' || param[param.size() - 1] != '"') {
            error = true;
            return;
        } else {
            ascii_string = param.substr(1, param.size() - 2);
        }
    }
    else if(directive == WORD) {
        if(!check_symbol(param) && !check_literal(param)) {
            error = true;
            return;
        }
        symbols.push_back(param);
    }
    else if(directive == GLOBAL || directive == EXTERN) {
        if(check_symbol(param)) {
            symbols.push_back(param);
        } else {
            error = true;
            return;
        }
    }
    else if(directive == SKIP) {
        if(check_literal(param)) {
            operand_literal = convert_literal(param);
        } else {
            error = true;
            return;
        }
    }
    else if(directive == SECTION) {
        if(check_symbol(param)) {
            operand_symbol = param;
        } else {
            error = true;
            return;
        }
    }
    else if(directive == EQU) {
        if(operand_symbol == "") {
            if(check_symbol(param)) {
                operand_symbol = param;
            } else {
                error = true;
                return;
            }
        } else {
            if(check_expression(param)) {
                set_expression(param);
            } else {
                error = true;
                return;
            }
        }
    }
    else {
        error = true;
    }
}

bool ParsedLine::check_symbol(string &str) {
    regex regex_expression("[a-zA-Z][a-zA-Z0-9_]*");
    return regex_match(str, regex_expression);
}

bool ParsedLine::check_literal(string &str) {
    regex regex_expression_hex("0x[0-9a-fA-F]+");
    regex regex_expression_dec("[0-9]+");
    return regex_match(str, regex_expression_dec) || regex_match(str, regex_expression_hex);
}

bool ParsedLine::check_expression(string &str) {
    int l = 0, r = 0, len = str.size();
    if(str[0] == '-') l = r = 1;
    while(l < len) {
        while(r < len && str[r] != '-' && str[r] != '+') r++;
        string op = str.substr(l, r - l);
        strip(op);
        if(op == "" || !check_symbol(op) && !check_literal(op)) {
            return false;
        }
        l = ++r;
    }
    return true;
}

void ParsedLine::set_expression(string &str) {
    int l = 0, r = 0, len = str.size();
    bool sign = false;
    if(str[0] == '-') {
        sign = true;
        l = r = 1;
    }
    while(l < len) {
        while(r < len && str[r] != '-' && str[r] != 'l') r++;
        string op = str.substr(l, r - l);
        strip(op);
        expression.push_back({op, sign});
        sign = (str[r] == '-');
        l = r++;
    }

}

int ParsedLine::convert_literal(string &str) {
    // assumes that str is valid literal
    int number = 0;
    if(str.size() > 2 && str[1] == 'x') {
        for(int i = 2; i < str.size(); i++) {
            if(str[i] >= '0' && str[i] <= '9') number = number * 16 + str[i] - '0';
            else if(str[i] >= 'a' && str[i] <= 'f') number = number * 16 + 10 + str[i] - 'a';
            else number = number * 16 + 10 + str[i] - 'A';
        }
    } else {
        for(int i = 0; i < str.size(); i++) {
            number = number * 10 + str[i] - '0';
        }
    }
    return number;
}

ParsedLine::ParsedLine(string line) {
    clean_comment(line);
    if(line == "") {
        empty = true;
        return;
    }
    int l = 0, r = 0, len = line.size();
    vector<string> tokens;
    char delimiter1 = ' ', delimiter2 = '\t';
    bool labels_over = true;
    while(l < len) {
        while(r < len && line[r] != delimiter1 && line[r] != delimiter2) {
            if(delimiter1 == ' ' && line[r] == ':') labels_over = false;
            r++;
        }
        if(l != r) {
            string token = line.substr(l, r - l);
            strip(token);
            tokens.push_back(token);
            if(labels_over) { delimiter1 = ','; delimiter2 = ','; }
            labels_over = true;
        }
        l = ++r;
    }
    int i = 0;
    len = tokens.size();
    while(i < len && tokens[i][tokens[i].size() - 1] == ':') {
        tokens[i].pop_back();
        if(!check_symbol(tokens[i])) {
            error = true;
            return;
        }
        this->labels.push_back(tokens[i]);
        this->type = LABEL;
        ++i;
    }
    if(i < len) {
        set_command(tokens[i]);
        ++i;
        if(type == NONE_TYPE) {
            error = true;
            return;
        }
    }
    while(i < len) {
        add_param(tokens[i]);
        i++;
    }

    if(!error) final_check();
}

void ParsedLine::final_check() {
    if(type == NONE_TYPE) {
        error = true;
    } else if(type == LABEL) {
        error = (labels.size() == 0);
    } else if(type == DIRECTIVE) {
        switch (directive) {
            case GLOBAL:
            case EXTERN:
                if(symbols.size() == 0) error = true;
                break;
            case SECTION:
                if(operand_symbol == "") error = true;
                break;
            case WORD:
                if(symbols.size() == 0) error = true;
                break;
            case SKIP:
                if(operand_literal == -1) error = true;
                break;
            case ASCII:
                if(ascii_string == "") error = true;
                break;
            case EQU:
                if(operand_symbol == "" || expression.size() == 0) error = true;
                break;
        }
    } else if(type == INSTRUCTION) {
        switch(instruction) {
            case JMP:
            case JEQ:
            case JNE:
            case JGT:
            case CALL:
                if(param_count != 1 || addressing == NONE_ADDR) error = true;
                break;
            case XCHG:
            case ADD:
            case SUB:
            case MUL:
            case DIV:
            case CMP:
            case AND:
            case OR:
            case XOR:
            case TEST:
            case SHL:
            case SHR:
                if(param_count != 2 || reg_dst == NONE_REG || reg_src == NONE_REG) error = true;
                break;
            case NOT:
            case PUSH:
            case POP:
            case INT:
                if(param_count != 1 || reg_dst == NONE_REG) error = true;
                break;
            case LDR:
            case STR:
                if(param_count != 2 || reg_dst == NONE_REG || addressing == NONE_ADDR) error = true;
                break;
            case IRET:
            case RET:
            case HALT:
                if(param_count != 0) error = true;
                break;
        }
    }
}


