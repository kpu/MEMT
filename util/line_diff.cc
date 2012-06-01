#include <boost/iostreams/copy.hpp>

#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << argv[0] << " file1 file2" << std::endl;
    return 1;
  }
  std::ifstream file1(argv[1]), file2(argv[2]);
  std::string line1, line2;
  unsigned long long lineno = 0;
  for (; getline(file1, line1) && getline(file2, line2); ++lineno) {
    if (line1 != line2) {
      std::cout << lineno << '\n' << line1 << '\n' << line2 << '\n';
    }
  }
  if (!file1 && !file1.eof()) {
    std::cerr << "Error reading " << argv[1] << std::endl;
    return 2;
  }
  if (!file2 && !file2.eof()) {
    std::cerr << "Error reading " << argv[2] << std::endl;
    return 3;
  }
  std::istream *remaining = file1 ? &file1 : &file2;
  if (*remaining) {
    std::cout << "Remaining:" << '\n';
    boost::iostreams::copy(*remaining, std::cout);
  }
  if (!remaining->eof()) {
    std::cerr << "Error reading." << std::endl;
  }
}
