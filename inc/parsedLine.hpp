#ifndef _PARSED_LINE_H_
#define _PARSED_LINE_H_

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

class ParsedLine {
    vector<string> labels;
    string instruction;
    string directive;
    vector<string> params;

    bool parse_failed = false;
    bool parse_over = false;
public:
    static const vector<string> directives;
    static const vector<string> instructions;

    void parseLine(ifstream& ifs);

    static void l_strip(string& str);
    static bool check_name(string str);
    static bool check_directive(string str);
    static bool check_instruction(string str);

    bool check_parse_failed();
    bool check_parse_over();

    friend ostream& operator<<(ostream& os, const ParsedLine& pl);
};

#endif