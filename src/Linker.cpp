#include "../inc/Linker.hpp"
#include "../inc/ParsedLine.hpp"
#include "../inc/Assembler.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>

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

  if(found_hex && found_relocatable) {
      error = true;
      error_message = "Can't link with both -hex and -relocatable options";
      return;
  }
  if(!found_hex && !found_relocatable) {
      error = true;
      error_message = "You need to provide one of these options: -hex, -relocatable";
      return;
  }

  for(string input_filename : filenames) {
    // input_filename = "tests/" + input_filename;
      Assembler assembler;
      ifstream rf(input_filename, ios::out | ios::binary);
      rf >> assembler;

      bool new_section = false;
      for(auto& entry : assembler.relocation_table) {
          if(!relocation_tables.count(entry.first)) {
              relocation_tables[entry.first] = {};
              new_section = true;
          }
          for(auto& row : entry.second) {
              relocation_tables[entry.first].push_back(row);
              if(!new_section) {
                  relocation_tables[entry.first].back().offset += symbol_table[entry.first].size;
              }
              if(symbol_table.count(relocation_tables[entry.first].back().symbol)
              && symbol_table[relocation_tables[entry.first].back().symbol].is_section) {
                  relocation_tables[entry.first].back().addend += symbol_table[relocation_tables[entry.first].back().symbol].size;
              }
          }
      }

      for(auto& entry : assembler.symbol_table) {
          if(entry.second.is_global) {
            if(entry.second.section == "__und__") {
                if(!symbol_table.count(entry.first)) {
                    symbol_table[entry.first] = entry.second;
                }
            } else {
                if(symbol_table.count(entry.first) && symbol_table[entry.first].section != "__und__") {
                    error = true;
                    error_message = "Multiple definition of symbol " + entry.first;
                    return;
                } else {
                    symbol_table[entry.first] = entry.second;
                    if(symbol_table.count(entry.second.section)) {
                        symbol_table[entry.first].value += symbol_table[entry.second.section].size;
                    }
                }
            }
          }
      }
      for(auto& entry: assembler.symbol_table) {
          if(entry.second.is_section) {
              if(symbol_table.count(entry.first)) {
                  symbol_table[entry.first].size += entry.second.size;
              } else {
                  symbol_table[entry.first] = entry.second;
              }
          }
      }

      for(auto& content : assembler.section_content) {
          if(section_content.count(content.first)) {
              section_content[content.first].insert(section_content[content.first].end(), content.second.begin(), content.second.end());
          } else {
              section_content[content.first] = content.second;
          }
      }
  }

  if(is_hex) {
      for(auto& entry : symbol_table) {
          if(entry.second.section == "__und__") {
              error = true;
              error_message = "Symbol " + entry.first + " is unresolved";
              return;
          }
      }

      int next_section = 0;
      for(auto& entry1 : place) {
          for(auto& entry2 : place) {
              if(entry1 != entry2) {
                  if(entry1.second <= entry2.second && entry1.second + symbol_table[entry1.first].size > entry2.second) {
                      error = true;
                      error_message = "Sections " + entry1.first + " and " + entry2.first + " will overlap";
                      return;
                  }
              }
          }
          if(entry1.second + symbol_table[entry1.first].size > next_section) {
              next_section = entry1.second + symbol_table[entry1.first].size;
          }
      }

      // loader
      for(auto& entry : section_content) {
          if(place.count(entry.first)) continue;
          place[entry.first] = next_section;
          next_section += symbol_table[entry.first].size;
      }
      vector<unsigned char> hex_content(next_section, 0);

      for(auto& entry : section_content) {
          for(int i = 0; i < entry.second.size(); i++) {
              unsigned char byte = entry.second[i];
              hex_content[place[entry.first] + i] = byte;
          }
      }

      for(auto& entry : relocation_tables) {
          string section = entry.first;
          for(auto& row : entry.second) {
            if(place.count(row.symbol)) {
              hex_content[place[section] + row.offset] = (place[row.symbol] + row.addend) >> 8;
              hex_content[place[section] + row.offset + 1] = (place[row.symbol] + row.addend) & 255;
            }
            else {
                hex_content[place[section] + row.offset] = (place[symbol_table[row.symbol].section] + symbol_table[row.symbol].value) >> 8;
                hex_content[place[section] + row.offset + 1] = (place[symbol_table[row.symbol].section] + symbol_table[row.symbol].value) & 255;
            }
          }
      }

      ofstream hex_output(output_filename, ios::out | ios::binary);
      for(int i = 0; i < hex_content.size(); i++) {
          hex_output << setw(2) << setfill('0') << hex << +hex_content[i] << setw(0) << " ";
          if((i + 1) % 8 == 0) hex_output << endl;
          if(i < 256) {
            cout << +hex_content[i] << " ";
            if((i + 1) % 8 == 0) cout << endl;
        }
      }
  }
  else {
    ofstream rel_output(output_filename, ios::out | ios::binary);
    rel_output << symbol_table.size() << endl;
    for(auto& entry : symbol_table) {
        rel_output << entry.first << " " << 
                entry.second.value << " " << 
                entry.second.size << " " << 
                entry.second.section << " " << 
                entry.second.is_section << " " << 
                entry.second.is_global << endl;
    }
    
    rel_output << relocation_tables.size() << endl;
    for(auto& entry : relocation_tables) {
        rel_output << entry.first << endl << entry.second.size() << endl;
        for(auto& row : entry.second) {
            rel_output << row.offset << " " << row.symbol << " " << row.addend << endl;
        }
    }

    rel_output << section_content.size() << endl;
    for(auto& entry : section_content) {
        rel_output << entry.first << endl << entry.second.size() << endl;
        for(auto& byte : entry.second) {
            rel_output << +byte << " ";
        }
        rel_output << endl;
    }

  }

}