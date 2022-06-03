#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <ifstream>

#include "parsedLine.hpp"

using namespace std;

class Parser {
  static Parser parser_instance;
  ifstream* file_stream = nullptr;
  bool eof = false;
  bool error = false;
public:
  static Parser getInstance();
  void setStream(string filename);
  ParsedLine nextLine();
};

#endif