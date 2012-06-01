#include "util/utf8.hh"

#include <iostream>

int main() {
  std::string line, lower;
  while (getline(std::cin, line)) {
    utf8::ToLower(line, lower);
    std::cout << lower << '\n';
  }
  if (!std::cin.eof()) {
    std::cerr << "Some error other than EOF" << std::endl;
    return 1;
  }
  return 0;
}
