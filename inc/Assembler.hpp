#ifndef _ASEMBLER_H_
#define _ASEMBLER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

using namespace std;

class Assembler {
    static Assembler assembler_instance;
    Assembler() {}
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

        relocation_entry() {}
        relocation_entry(int offset, string symbol, int addend)
            : offset(offset), symbol(symbol), addend(addend) {}
    };

    class forward_link_entry {
    public:
        string section;
        int location_counter;
        string symbol;

        forward_link_entry() {}
        forward_link_entry(string section, int location_counter, string symbol)
                : section(section), location_counter(location_counter), symbol(symbol) {}
    };

    unordered_map<string, symbol_table_entry> symbol_table;
    unordered_map<string, vector<relocation_entry>> relocation_table;
    unordered_map<string, vector<char>> section_content;                // bytes
    vector<forward_link_entry> forward_link;
    unordered_map<string, vector<pair<string, bool>>> unresolved_symbols; // map of unresolved symbols .equ
                                                    // vector is an array of symbols/literals and their sign

    bool check_solvable(vector<pair<string, bool>>& expression);
    int expression_value(vector<pair<string, bool>>& expression);

public:
    static Assembler& getInstance();
    void assemble();
    void set_file(string filename);

    friend ostream& operator<<(ostream& os, Assembler& as);
};


#endif
