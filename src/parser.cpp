#include "../inc/parser.hpp"

Parser Parser::getInstance() {
  return parser_instance;
}

void Parser::setStream(string filename) {
  if(this->file_stream != nullptr) {
    this->file_stream->close();
    delete this->file_stream;
  }
  this->file_stream = new fstream(filename);
}

ParsedLine Parse::nextLine() {
  ParsedLine pl;
  pl.parseLine(*this->file_stream);
  this->eof = this->file_stream.eof();
  return pl;
}
