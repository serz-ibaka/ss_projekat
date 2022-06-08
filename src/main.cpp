#include <iostream>
#include "../inc/Assembler.hpp"

int main() {
    Assembler& assembler = Assembler::getInstance();
    assembler.set_file("tests/test.s");
    assembler.assemble();
    cout << assembler;
    return 0;
}
