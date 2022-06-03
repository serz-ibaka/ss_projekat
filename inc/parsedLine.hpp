#ifndef _PARSED_LINE_H_
#define _PARSED_LINE_H_

#include <string>
#include <vector>
#include <iostream>
#include <ifstream>

using namespace std;

class ParsedLine {
  vector<string> labels;
  string instruction;
  string directive;
  vector<string> params;
public:
  void parseLine(ifstream ifs);

};

#endif