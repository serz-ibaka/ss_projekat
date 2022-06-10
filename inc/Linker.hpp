#ifndef _LINKER_H_
#define _LINKER_H_

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class Linker {

  vector<string> filenames;
  bool is_hex = false;
  unordered_map<string, int> place;
  bool error = false;
  string error_message;
  string output_filename;


  class symbol_table_entry {
  public:
      int value;
      bool is_global = false;
      string section = "__und__";
      int size = 0;
      bool is_section = false;

      symbol_table_entry() {}
      symbol_table_entry(int value, string section, bool is_global = false, bool is_section = false, int size = 0)
          : value(value), section(section), is_global(is_global), is_section(is_section), size(size) {}
  };

  class relocation_entry {
  public:
      int offset;
      string symbol;
      int addend;

      relocation_entry() {}
      relocation_entry(int offset, string symbol, int addend)
          : offset(offset), symbol(symbol), addend(addend) {}
  };

  vector<unordered_map<string, symbol_table_entry>> symbol_tables;
  unordered_map<string, vector<relocation_entry>> relocation_tables;
  unordered_map<string, vector<unsigned char>> section_content;

public:

  void fetch_all_data(int argc, char* argv[]);


};

#endif