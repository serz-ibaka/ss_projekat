#include "../inc/Parser.hpp"

Parser Parser::parser_instance;

Parser& Parser::get_instance() {
    return parser_instance;
}

bool Parser::finished_parsing() {
    return end;
}

void Parser::set_file(string filename) {
    ifs.open(filename);
}

ParsedLine Parser::nextLine() {
    string line;

    if(!getline(ifs, line)) {
        end = true;
    }
    return ParsedLine(line);
}
