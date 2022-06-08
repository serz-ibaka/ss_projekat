#ifndef _PARSER_H
#define _PARSER_H

#include <fstream>
#include "ParsedLine.hpp"

using namespace std;

class Parser {
    ifstream ifs;
    string filename;
    static Parser parser_instance;
    Parser() {}
    bool end = false;

public:
    static Parser& get_instance();
    void set_file(string filename);
    ParsedLine nextLine();
    bool finished_parsing();
};


#endif
