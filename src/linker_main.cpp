#include "../inc/Linker.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
  Linker linker;
  linker.fetch_all_data(argc, argv);
  cout << linker.get_error_message();
}