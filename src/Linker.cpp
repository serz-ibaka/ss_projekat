#include "../inc/Linker.hpp"
#include "../inc/ParsedLine.hpp"
#include "../inc/Assembler.hpp"

void Linker::fetch_all_data(int count, char* arguments[]) {
  vector<string> argv;
  for(int i = 1; i < count; i++) {
    argv.push_back(string(arguments[i]));
  }
  int output_index = -2;
  bool found_hex = false, found_relocatable = false;
  for(int i = 0; i < argv.size(); i++) {
    if(argv[i] == "-hex") {
      is_hex = true;
      found_hex = true;
    } else if(argv[i] == "-relocatable") {
      is_hex = false;
      found_relocatable = true;
    } else if(argv[i] == "-o") {
      output_index = i;
    } else if(argv[i].substr(1, 5) == "place") {
      int index = 7, len = 0;
      string section;
      while(index + len < argv[i].size() && argv[i][index + ++len] != '@');
      section = argv[i].substr(index, len);
      index = index + len + 1;

      string s_place = argv[i].substr(index);
      int place = ParsedLine::convert_literal(s_place);

      this->place[section] = place;

    } else {
      if(i == output_index + 1) {
        output_filename = argv[i];
      } else {
        filenames.push_back(argv[i]);
      }
    }
  }
}
