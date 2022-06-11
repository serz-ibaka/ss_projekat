#include "../inc/Assembler.hpp"
#include <fstream>

// asembler -o izlaz.o ulaz.s

int main(int argc, char* argv[]) {
    if(argc != 4 || string(argv[1]) != "-o") { return 0; }
    string input_filename = "tests/" + string(argv[3]);
    string output = argv[2];

    Assembler assembler(input_filename);
    assembler.assemble();
    
    ofstream wf(output, ios::out | ios::binary);
    wf << assembler;

    assembler.print_assembled();

    return 0;
}