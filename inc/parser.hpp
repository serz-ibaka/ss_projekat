#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <fstream>

#include "parsedLine.hpp"

using namespace std;

class Parser {
    static Parser parser_instance;
    ifstream file_stream;
    bool eof = false;
    bool error = false;
    Parser() {}
public:
    static Parser& getInstance();
    void setStream(string filename);
    ParsedLine nextLine();
    bool check_end_of_file();
};

#endif