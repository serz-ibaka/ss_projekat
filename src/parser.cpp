#include "../inc/parser.hpp"

Parser::Parser Parser::getInstance() {
  return parser_instance;
}

void Parser::setStream(string filename) {
  if(this->file_stream != nullptr) {
    this->file_stream->close;
    delete this->file_stream;
  }
  this->file_stream = new fstream(filename);
}



/*
class Parser {
  static Parser parser;
  ifstream file_stream;
  Parser();
public:
  Parser getInstance();
  void setStream(string filename);
};
*/