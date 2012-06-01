/* Pick a short sentence pair containing all alignment types. */
#include "MEMT/Input/AlignType.hh"
#include "MEMT/Input/Input.hh"
#include "MEMT/Input/Format.hh"
#include "MEMT/Input/ReadFromJava.hh"

#include <iostream>
#include <string>
#include <vector>

int main() {
  const input::AlignType kWanted = input::AL_EXACT | input::AL_WN_SYNONYMY | input::AL_SNOWBALL_STEM | input::AL_PARAPHRASE;
  const unsigned int kMaxLength = 12;
  input::Input input;
  for (unsigned int v = 0; ; ++v) {
    try {
      input::ReadFromJava(std::cin, input);
    }
    catch (std::ios_base::failure &f) {
      if (std::cin.eof()) break;
      throw;
    }
    for (unsigned int e = 0; e < input.engines.size(); ++e) {
      for (unsigned int f = e + 1; f < input.engines.size(); ++f) {
        input::AlignType got = 0;
        for (unsigned int w = 0; w < input.engines[e].words.size(); ++w) {
          got |= input.GetWord(e, w).alignments.Ask(f).type;
        }
        if (((got & kWanted) == kWanted) && (input.engines[e].words.size() < kMaxLength) && (input.engines[f].words.size() < kMaxLength)) {
          LaTeXAlignment(std::cout, "System 1", input.engines[e], "System 2", input.engines[f]);
        }
      }
    }
  }
}
