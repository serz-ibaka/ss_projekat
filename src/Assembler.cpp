#include "../inc/Assembler.hpp"
#include "../inc/Parser.hpp"

#include <iostream>
#include <iomanip>

bool Assembler::check_solvable(vector<pair<string, bool>>& expression) {
    for(auto& element: expression) {
        if(ParsedLine::check_literal(element.first)) continue;
        if(symbol_table.count(element.first)) continue;
        return false;
    }
    return true;
}

int Assembler::expression_value(vector<pair<string, bool>>& expression) {
    int value = 0;
    for(auto& element: expression) {
        if(ParsedLine::check_literal(element.first)) {
            value += ParsedLine::convert_literal(element.first) * (element.second ? -1 : 1);
        }
        else {
            value += symbol_table[element.first].value * (element.second ? -1 : 1);
        }
    }
    return value;
}

void Assembler::assemble() {
    location_counter = 0;
    int section_counter = -1;
    Parser& parser = Parser::get_instance();
    parser.set_file(filename);

    symbol_table[current_section] = symbol_table_entry(0, current_section);

    while(!parser.finished_parsing()) {
        ParsedLine parsed_line = parser.nextLine();
        if(parsed_line.empty) continue;
        if(parsed_line.error) {
            cout << "Greska u parsiranju! ";
            error = true;
            return;
        }

        for(string label : parsed_line.labels) {
            if(symbol_table.count(label)) {
                symbol_table[label].section = current_section;
                symbol_table[label].value = location_counter;
            } else {
                symbol_table[label] = symbol_table_entry(location_counter, current_section);
            }
        }
        if(parsed_line.type == ParsedLine::LABEL) continue;
        if(parsed_line.directive == ParsedLine::END) {
            symbol_table[current_section].size = location_counter - symbol_table[current_section].value;
            break;
        }
        if(parsed_line.directive == ParsedLine::GLOBAL) {
            for(string symbol : parsed_line.symbols) {
                symbol_table[symbol] = symbol_table_entry(0, "__tbd__", true);
            }
        }
        else if(parsed_line.directive == ParsedLine::EXTERN) {
            for(string symbol : parsed_line.symbols) {
                symbol_table[symbol] = symbol_table_entry(0, "__und__", true);
            }
        }
        else if(parsed_line.directive == ParsedLine::SECTION) {
            if(current_section != "__und__") {
                symbol_table[current_section].size = location_counter - symbol_table[current_section].value;
            }
            current_section = parsed_line.operand_symbol;
            location_counter = 0;
            symbol_table[current_section] = symbol_table_entry(0, current_section, false, true);
            section_content.push_back({current_section, {}});
            relocation_table.push_back({current_section, {}});
            section_counter++;
        }
        else if(parsed_line.directive == ParsedLine::SKIP) {
            for(int i = 0; i < parsed_line.operand_literal; i++) {
                section_content[section_counter].second.push_back(0);
            }
            location_counter += parsed_line.operand_literal;
        }
        else if(parsed_line.directive == ParsedLine::ASCII) {
            for(char c : parsed_line.ascii_string) {
                section_content[section_counter].second.push_back(c);
                location_counter++;
            }
        }
        else if(parsed_line.directive == ParsedLine::WORD) {
            for(string symbol : parsed_line.symbols) {
                if(ParsedLine::check_literal(symbol)) {
                    // TODO : error number too big
                    int value = ParsedLine::convert_literal(symbol);
                    int low = value & 255;
                    int high = value >> 8;
                    section_content[section_counter].second.push_back(high);
                    section_content[section_counter].second.push_back(low);
                }
                else {
                    if(symbol_table.count(symbol)) {
                        int offset = location_counter;
                        string sym;
                        int addend;
                        if(symbol_table[symbol].is_global) {
                            sym = symbol;
                            addend = 0;
                        } else {
                            sym = symbol_table[symbol].section;
                            addend = symbol_table[symbol].value;
                        }
                        relocation_table[section_counter].second.push_back(relocation_entry(offset, sym, addend));
                    } else {
                        forward_link.push_back(forward_link_entry(current_section, location_counter, symbol));
                    }
                    section_content[section_counter].second.push_back(0);
                    section_content[section_counter].second.push_back(0);
                }
                location_counter += 2;
            }
        }
        else if(parsed_line.directive == ParsedLine::EQU) {
            if(check_solvable(parsed_line.expression)) {
                if(symbol_table.count(parsed_line.operand_symbol)) {
                    symbol_table[parsed_line.operand_symbol].section = "__abs__";
                    symbol_table[parsed_line.operand_symbol].value = expression_value(parsed_line.expression);
                } else {
                    symbol_table[parsed_line.operand_symbol] = symbol_table_entry(expression_value(parsed_line.expression), "__abs__");
                }
            } else {
                unresolved_symbols.insert({parsed_line.operand_symbol, parsed_line.expression});
            }
        }
        else if(parsed_line.instruction == ParsedLine::HALT) {
            section_content[section_counter].second.push_back(0);
            location_counter++;
        }
        else if(parsed_line.instruction == ParsedLine::IRET) {
            section_content[section_counter].second.push_back(32);
            location_counter++;
        }
        else if(parsed_line.instruction == ParsedLine::RET) {
            section_content[section_counter].second.push_back(64);
            location_counter++;
        }
        else if(parsed_line.instruction == ParsedLine::INT) {
            section_content[section_counter].second.push_back(16);
            section_content[section_counter].second.push_back(15 + (parsed_line.reg_dst - 1 << 4));
            location_counter += 2;
        }
        else if(parsed_line.instruction == ParsedLine::XCHG) {
            section_content[section_counter].second.push_back(96);
            section_content[section_counter].second.push_back(parsed_line.reg_src - 1 + (parsed_line.reg_dst - 1 << 4));
            location_counter += 2;
        }
        else if(parsed_line.instruction >= ParsedLine::ADD && parsed_line.instruction <= ParsedLine::CMP) {
            section_content[section_counter].second.push_back(112 + parsed_line.instruction - ParsedLine::ADD);
            section_content[section_counter].second.push_back(parsed_line.reg_src - 1 + (parsed_line.reg_dst - 1 << 4));
            location_counter += 2;
        }
        else if(parsed_line.instruction == ParsedLine::NOT) {
            section_content[section_counter].second.push_back(128);
            section_content[section_counter].second.push_back(parsed_line.reg_dst - 1 << 4);
            location_counter += 2;
        }
        else if(parsed_line.instruction >= ParsedLine::AND && parsed_line.instruction <= ParsedLine::TEST) {
            section_content[section_counter].second.push_back(128 + parsed_line.instruction - ParsedLine::NOT);
            section_content[section_counter].second.push_back(parsed_line.reg_src - 1 + (parsed_line.reg_dst - 1 << 4));
            location_counter += 2;
        }
        else if(parsed_line.instruction == ParsedLine::SHL || parsed_line.instruction == ParsedLine::SHR) {
            section_content[section_counter].second.push_back(144 + parsed_line.instruction - ParsedLine::SHL);
            section_content[section_counter].second.push_back(parsed_line.reg_src - 1 + (parsed_line.reg_dst - 1 << 4));
            location_counter += 2;
        }
        else if(parsed_line.instruction == ParsedLine::PUSH) {
            section_content[section_counter].second.push_back(176);
            section_content[section_counter].second.push_back(ParsedLine::SP - 1 + (parsed_line.reg_dst - 1 << 4));
            section_content[section_counter].second.push_back(18);
            location_counter += 3;
        }
        else if(parsed_line.instruction == ParsedLine::POP) {
            section_content[section_counter].second.push_back(160);
            section_content[section_counter].second.push_back(ParsedLine::SP - 1 + (parsed_line.reg_dst - 1 << 4));
            section_content[section_counter].second.push_back(66);
            location_counter += 3;
        }
        else if(parsed_line.instruction >= ParsedLine::CALL && parsed_line.instruction <= ParsedLine::JGT
                || parsed_line.instruction == ParsedLine::LDR || parsed_line.instruction == ParsedLine::STR) {
            // first byte
            int instr;
            if(parsed_line.instruction == ParsedLine::CALL) instr = 48;
            else if(parsed_line.instruction < ParsedLine::LDR) instr = 80 + parsed_line.instruction - ParsedLine::JMP;
            else instr = 10 + parsed_line.instruction - ParsedLine::LDR << 4;
            section_content[section_counter].second.push_back(instr);

            // second byte
            int reg = 240 + parsed_line.reg_dst - 1;
            if(parsed_line.reg_dst == ParsedLine::NONE_REG) {
                reg = 255;
            }
            if(parsed_line.instruction == ParsedLine::STR || parsed_line.instruction == ParsedLine::LDR) {
                reg = (parsed_line.reg_dst - 1 << 4) + parsed_line.reg_src - 1;
                if(parsed_line.reg_src == ParsedLine::NONE_REG) {
                    reg = (parsed_line.reg_dst - 1 << 4) + 15;
                }
            }
            section_content[section_counter].second.push_back(reg);

            // third byte
            int addressing = 0;
            if(parsed_line.addressing == ParsedLine::MEM_LIT || parsed_line.addressing == ParsedLine::MEM_SYM) {
                addressing = 4;
            }
            else if(parsed_line.addressing == ParsedLine::REG_IND_LIT || parsed_line.addressing == ParsedLine::REG_IND_SYM) {
                addressing = 3;
            }
            else if(parsed_line.addressing == ParsedLine::PCREL) {
                if(parsed_line.instruction == ParsedLine::LDR || parsed_line.instruction == ParsedLine::STR) {
                    addressing = 3;
                } else {
                    addressing = 5;
                }
            }
            else if(parsed_line.addressing == ParsedLine::REG_DIR) {
                addressing = 1;
            }
            else if(parsed_line.addressing == ParsedLine::REG_IND) {
                addressing = 2;
            }
            section_content[section_counter].second.push_back(addressing);

            location_counter += 3;

            // 3 bytes for REG_DIR and REG_IND
            // 4th and 5th byte
            if(parsed_line.addressing == ParsedLine::LIT
            || parsed_line.addressing == ParsedLine::MEM_LIT
            || parsed_line.addressing == ParsedLine::REG_IND_LIT) {
                section_content[section_counter].second.push_back(parsed_line.operand_literal >> 8);
                section_content[section_counter].second.push_back(parsed_line.operand_literal & 255);
                location_counter += 2;
            }
            else if(parsed_line.addressing == ParsedLine::SYM
            || parsed_line.addressing == ParsedLine::MEM_SYM
            || parsed_line.addressing == ParsedLine::REG_IND_SYM
            || parsed_line.addressing == ParsedLine::PCREL) {
                if(symbol_table.count(parsed_line.operand_symbol)) {
                    if(symbol_table[parsed_line.operand_symbol].section == "__abs__") {
                        section_content[section_counter].second.push_back(symbol_table[parsed_line.operand_symbol].value >> 8);
                        section_content[section_counter].second.push_back(symbol_table[parsed_line.operand_symbol].value & 255);
                        location_counter += 2;
                        continue;
                    }
                    else {
                        int offset = location_counter;
                        string sym;
                        int addend;
                        if(symbol_table[parsed_line.operand_symbol].is_global) {
                            sym = parsed_line.operand_symbol;
                            addend = 0;
                        } else {
                            sym = symbol_table[parsed_line.operand_symbol].section;
                            addend = symbol_table[parsed_line.operand_symbol].value;
                        }
                        relocation_table[section_counter].second.push_back(relocation_entry(offset, sym, addend));
                        if(parsed_line.addressing == ParsedLine::PCREL) {
                            relocation_table[section_counter].second.back().is_PC = true;
                            relocation_table[section_counter].second.back().addend -= 2;
                        }
                    }
                } else {
                    forward_link.push_back(forward_link_entry(current_section, location_counter, parsed_line.operand_symbol));
                    if(parsed_line.addressing == ParsedLine::PCREL) {
                        forward_link.back().is_PC = true;
                    }
                }

                section_content[section_counter].second.push_back(0);
                section_content[section_counter].second.push_back(0);
                location_counter += 2;
            }
        }
    }


    while(unresolved_symbols.size() > 0) {
        bool resolved = false;
        string deletion_symbol;
        for(auto& entry : unresolved_symbols) {
            if (check_solvable(entry.second)) {
                if (symbol_table.count(entry.first)) {
                    symbol_table[entry.first].value = expression_value(entry.second);
                    symbol_table[entry.first].section = "__abs__";
                } else {
                    symbol_table[entry.first] = symbol_table_entry(expression_value(entry.second), "__abs__");
                }
                deletion_symbol = entry.first;
                resolved = true;
                break;
            }
        }
        if(!resolved) {
            cout << "Greska pri asembliranju - nisu razreseni svi apsolutni simboli" << endl;
            return;
        }
        unresolved_symbols.erase(deletion_symbol);
    }

    for(auto& flink : forward_link) {
        if(symbol_table.count(flink.symbol)) {
            if(symbol_table[flink.symbol].section == "__abs__") {
                section_content[section_counter].second[flink.location_counter] = symbol_table[flink.symbol].value >> 8;
                section_content[section_counter].second[flink.location_counter + 1] = symbol_table[flink.symbol].value & 255;
            } else {
                int offset = flink.location_counter;
                string sym;
                int addend;
                if(symbol_table[flink.symbol].is_global) {
                    sym = flink.symbol;
                    addend = 0;
                } else {
                    sym = symbol_table[flink.symbol].section;
                    addend = symbol_table[flink.symbol].value;
                }
                for(int i = 0; i < relocation_table.size(); i++) {
                    if(relocation_table[i].first == flink.section) {
                        relocation_table[i].second.push_back(relocation_entry(offset, sym, addend));
                        if(flink.is_PC) {
                            relocation_table[i].second.back().is_PC = true;
                            relocation_table[i].second.back().addend -= 2;
                        }
                        break;
                    }
                }
            }
            
        } else {
            cout << "Greska pri asembliranju - nisu razresena sva obracanja unapred" << endl;
            cout << "Simbol " << flink.symbol << endl;
        }
    }
}

ostream& operator<<(ostream& os, Assembler& as) {
    os << as.symbol_table.size() << endl;
    for(auto& entry : as.symbol_table) {
        os << entry.first << " " << 
                entry.second.value << " " << 
                entry.second.size << " " << 
                entry.second.section << " " << 
                entry.second.is_section << " " << 
                entry.second.is_global << endl;
    }
    
    os << as.relocation_table.size() << endl;
    for(auto& entry : as.relocation_table) {
        os << entry.first << endl << entry.second.size() << endl;
        for(auto& row : entry.second) {
            os << row.offset << " " << row.is_PC << " " << row.symbol << " " << row.addend << endl;
        }
    }

    os << as.section_content.size() << endl;
    for(auto& entry : as.section_content) {
        os << entry.first << endl << entry.second.size() << endl;
        for(auto& byte : entry.second) {
            os << +byte << " ";
        }
        os << endl;
    }

    return os;
}

istream& operator>>(istream& is, Assembler& as) {
    int sym_tab_size;
    is >> sym_tab_size;
    for(int i = 0; i < sym_tab_size; i++) {
        string name, section;
        int value, size;
        bool is_section, is_global;
        is >> name >> value >> size >> section >> is_section >> is_global;
        as.symbol_table[name] = Assembler::symbol_table_entry(value, section, is_global, is_section, size);
    }

    int num_relocation_tables;
    is >> num_relocation_tables;
    for(int i = 0; i < num_relocation_tables; i++) {
        string section;
        int size;
        is >> section >> size;
        as.relocation_table.push_back({ section, {}});
        for(int j = 0; j < size; j++) {
            int offset, addend, is_PC;
            string symbol;
            is >> offset >> is_PC >> symbol >> addend;
            as.relocation_table[i].second.push_back(Assembler::relocation_entry(offset, symbol, addend));
            as.relocation_table[i].second.back().is_PC = (is_PC == 1);
        }
    }

    int num_sections;
    is >> num_sections;
    for(int i = 0; i < num_sections; i++) {
        string section;
        int size;
        is >> section >> size;
        as.section_content.push_back({section, {}});
        for(int j = 0; j < size; j++) {
            int byte;
            is >> byte;
            as.section_content[i].second.push_back((char)byte);
        }
    }

    return is;
}
