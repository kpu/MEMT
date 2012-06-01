#include <iostream>

#include "MEMT/Input/Config.hh"
#include "MEMT/Input/Format.hh"
#include "MEMT/Input/Input.hh"
#include "MEMT/Input/ReadFromJava.hh"
#include "util/numbers.hh"

using namespace std;

int main() {
  input::Input input;
  input::Config config;
  config.lowercase_before_lm = false;
  config.transitive = false;
  unsigned int sentence_num = 0;
  while (1) {
    // TODO: command line option for number of systems.
    try {
      input::ReadFromJava(config, cin, input, 0);
    }
    catch (std::ios_base::failure &f) {
      break;
    }
    if (input.engines.size() < 2) {
      std::cerr << "Not enough engines." << std::endl;
      continue;
    }
    LaTeXAlignment(std::cout, "First", input.engines[0], "Second", input.engines[1]);
    ++sentence_num;
  }
  return 0;
}
