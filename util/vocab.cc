#include <boost/unordered_set.hpp>

#include <iostream>
#include <string>

int main() {
  boost::unordered_set<std::string> vocab;
  std::string word;
  while (std::cin >> word) {
    if (vocab.insert(word).second) std::cout << word << '\n';
  }
  if (!std::cin.eof()) {
    std::cerr << "Error reading" << std::endl;
    return 1;
  }
  return 0;
}
