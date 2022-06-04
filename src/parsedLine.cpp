#include "../inc/parsedLine.hpp"
#include <regex>
#include <algorithm>
#include <sstream>


const vector<string> ParsedLine::directives = {
        "global", "extern", "section",
        "word", "skip", "ascii", "equ", "end"
};

const vector<string> ParsedLine::instructions = {
        "halt", "int", "iret", "call", "ret",
        "jmp", "jeq", "jne", "jgt",
        "push", "pop",
        "xchg", "add", "sub", "mul", "div", "cmp",
        "not", "and", "or", "xor", "test",
        "shl", "shr",
        "ldr", "str"
};

void ParsedLine::l_strip(string& str) {
    int i = 0;
    for(; i < str.size() && str[i] == ' '; i++);
    str = str.substr(i);
    for(i = str.size() - 1; i >= 0 && str[i] == ' '; i--);
    str = str.substr(0, i + 1);
}

bool ParsedLine::check_name(string str) {
    regex str_expr("[a-zA-Z_][a-zA-Z_0-9]*");
    return regex_match(str,str_expr);
}

bool ParsedLine::check_directive(string str) {
    return find(directives.begin(), directives.end(), str) != directives.end();
}

bool ParsedLine::check_instruction(string str) {
    return find(instructions.begin(), instructions.end(), str) != instructions.end();
}

void ParsedLine::parseLine(ifstream& ifs) {
    if(ifs.eof()) {
        this->parse_over = true;
        return;
    }
    string line;
    while(getline(ifs, line)) {
        if(find(line.begin(), line.end(), '#') != line.end()) {
            int hashtag_index = find(line.begin(), line.end(), '#') - line.begin();
            line = line.substr(0, hashtag_index);
        }
        l_strip(line);
        while(find(line.begin(), line.end(), ':') != line.end()) {
            int semi_colon_index = find(line.begin(), line.end(), ':') - line.begin();
            string label = line.substr(0, semi_colon_index);
            if(!check_name(label)) {
                this->parse_failed = true;
                return;
            }
            this->labels.push_back(label);
            line = line.substr(semi_colon_index + 1);
            l_strip(line);
        }

        if(line.size() > 0 && line[0] == '.') {
            // directive
            int end_directive_index = line.find_first_of(" \n\t");
            if(end_directive_index == string::npos) {
                this->directive = line.substr(1);
            } else {
                this->directive = line.substr(1, end_directive_index - 1);
            }
            if(!check_directive(this->directive)) {
                this->parse_failed = true;
                return;
            }
            if(end_directive_index != string::npos) {
                line = line.substr(end_directive_index);
            } else {
                line = "";
            }
        } else if(line.size() > 0) {
            // instruction
            int end_instruction_index = line.find_first_of(" \n\t");
            if(end_instruction_index == string::npos) {
                this->instruction = line;
            } else {
                this->instruction = line.substr(0, end_instruction_index);
            }
            if(!check_instruction(this->instruction)) {
                this->parse_failed = true;
                return;
            }
            if(end_instruction_index != string::npos) {
                line = line.substr(end_instruction_index);
            } else {
                line = "";
            }
        } else {
            // only label
            continue;
        }
        l_strip(line);

        stringstream ss(line);
        string param;
        while (getline(ss, param, ',')) {
            l_strip(param);
            this->params.push_back(param);
        }

        if(directive != "" || instruction != "") {
            break;
        }
    }

    if(directive == "" && instruction == "") {
        this->parse_failed = true;
    }
}

bool ParsedLine::check_parse_failed() { return parse_failed; }

bool ParsedLine::check_parse_over() { return  parse_over; }

ostream& operator<<(ostream& os, const ParsedLine& pl) {
    if(pl.parse_failed) {
        return os << "Dogodila se greska pri parsiranju!!!" << endl;
    }
    os << "##########################################################" << endl << endl;
    if(pl.labels.size() > 0) {
        os << "Labels : ";
        for(string label: pl.labels) {
            os << label << " ";
        }
        os << endl;
    }
    if(pl.directive != "") {
        os << "Directive : " << pl.directive << endl;
    } else if(pl.instruction != "") {
        os << "Instruction: " << pl.instruction << endl;
    }

    if(pl.params.size() > 0) {
        os << "Params : ";
        for(string param : pl.params) {
            os << param << " ";
        }
        os << endl;
    }
    os << endl;
    os << "##########################################################" << endl;
    return os;
}

