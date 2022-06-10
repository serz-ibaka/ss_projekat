#include "../inc/Assembler.hpp"
#include <fstream>
#include <iostream>

int main() {

  Assembler assembler;
  ifstream rf("test.o", ios::in | ios::binary);
  rf >> assembler;
  assembler.print_assembled();  
}