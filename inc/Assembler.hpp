#ifndef _ASEMBLER_H
#define _ASEMBLER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

using namespace std;

class Linker;

class Assembler {
    string filename;
    bool error = false;

    int location_counter = 0;
    string current_section = "__und__";

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
        bool is_PC = false;

        relocation_entry() {}
        relocation_entry(int offset, string symbol, int addend)
            : offset(offset), symbol(symbol), addend(addend) {}
    };

    class forward_link_entry {
    public:
        string section;
        int location_counter;
        string symbol;
        bool is_PC = false;

        forward_link_entry() {}
        forward_link_entry(string section, int location_counter, string symbol)
                : section(section), location_counter(location_counter), symbol(symbol) {}
    };

    unordered_map<string, symbol_table_entry> symbol_table;
    vector<pair<string, vector<relocation_entry>>> relocation_table;
    vector<pair<string, vector<unsigned char>>> section_content;                // bytes
    vector<forward_link_entry> forward_link;
    unordered_map<string, vector<pair<string, bool>>> unresolved_symbols; // map of unresolved symbols .equ
                                                    // vector is an array of symbols/literals and their sign

    bool check_solvable(vector<pair<string, bool>>& expression);
    int expression_value(vector<pair<string, bool>>& expression);

public:
    Assembler(string filename) : filename(filename) {}
    Assembler() {}
    void assemble();

    void print_assembled();
    friend ostream& operator<<(ostream& os, Assembler& as);
    friend istream& operator>>(istream& is, Assembler& as);

    friend class Linker;
};


#endif
