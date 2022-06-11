#ifndef _LINKER_H_
#define _LINKER_H_

#include <string>
#include <unordered_map>
#include <vector>
#include "../inc/Assembler.hpp"

using namespace std;

class Linker {

  vector<string> filenames;
  bool is_hex = false;
  unordered_map<string, int> place;
  bool error = false;
  string error_message;
  string output_filename;

  unordered_map<string, Assembler::symbol_table_entry> symbol_table;
  unordered_map<string, vector<Assembler::relocation_entry>> relocation_tables;
  unordered_map<string, vector<unsigned char>> section_content;

public:

  void fetch_all_data(int argc, char* argv[]);



};

#endif