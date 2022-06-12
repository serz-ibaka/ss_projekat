#include "../inc/Assembler.hpp"
#include <fstream>
#include <iomanip>

int main(int argc, char* argv[]) {
  if(argc == 0) return 0;
  string input_filename(argv[1]);
  string output_filename = input_filename + "bjdump";
  ifstream rf(input_filename, ios::out | ios::binary);
  ofstream wf(output_filename, ios::out | ios::binary);
  
  int sym_tab_size;
  rf >> sym_tab_size;
  wf << "Symbol table: " << endl << endl;

  wf << "Value  Size   Type  Bind         Section            Name" << endl;
  for(int i = 0; i < sym_tab_size; i++) {
    string symbol, section;
    int value, is_section, is_global, size;

    rf >> symbol >> value >> size >> section >> is_section >> is_global;
    wf << " " << setw(4) << setfill('0') << hex << value;
    wf << setw(0) << "  " << setw(4) << setfill(' ') << size;
    wf << setw(0) << (is_global ? "  GLOB" : "   LOC");
    wf << (is_section ? "   SCTN  " : "  NOTYP  ");
    wf << setw(14) << section;
    wf << setw(0) << "  " << setw(14) << symbol << setw(0) << endl;
  }

  int rela_tables;
  rf >> rela_tables;
  for(int i = 0; i < rela_tables; i++) {
    wf << endl;
    string section;
    int rela_size;
    rf >> section >> rela_size;
    wf << ".rela." << section << endl << endl;
    wf << "Offset     Type          Symbol  Addend" << endl;
    for(int j = 0; j < rela_size; j++) {
      int offset, type, addend;
      string symbol;
      rf >> offset >> type >> symbol >> addend;
      wf << "  " << setw(4) << setfill('0') << hex << offset;
      wf << setw(0) << (type ? "   PC_REL  " : "  ABS_REL  ");
      wf << setw(14) << setfill(' ') << symbol << setw(0) << "  ";
      wf << setw(6) << dec << addend << setw(0) << endl;
    }

    wf << endl;
  }

  int section_contents;
  rf >> section_contents;
  for(int i = 0; i < section_contents; i++) {
    string section;
    int size;
    rf >> section >> size;
    wf << section << endl << endl;
    for(int j = 0; j < size; j++) {
      int byte;
      rf >> byte;
      wf << setw(2) << setfill('0') << hex << byte << setw(0) << " ";
      if((j + 1) % 8 == 0) wf << endl;
    }
    wf << endl << endl;
  }
  
}