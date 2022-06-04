#include "../inc/parser.hpp"

Parser Parser::parser_instance;

Parser& Parser::getInstance() {
    return parser_instance;
}

void Parser::setStream(string filename) {
    this->file_stream.open(filename);
}

ParsedLine Parser::nextLine() {
    ParsedLine pl;
    pl.parseLine(file_stream);
    if(pl.check_parse_failed()) this->error = true;
    if(pl.check_parse_over()) this->eof = true;
    return pl;
}

bool Parser::check_end_of_file() { return eof; }