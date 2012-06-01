#ifndef _MEMT_Input_Input_h
#define _MEMT_Input_Input_h

#include "MEMT/Strategy/Horizon/Config.hh"
#include "MEMT/Input/Location.hh"
#include "MEMT/Input/Word.hh"

#include "util/numbers.hh"

#include <vector>

namespace input {

// This doesn't really do much, but it's useful to pass around an object for
// an engine instead of the entire input and the engine number.
struct Engine {
  Engine() {}

  // Length of sentence, including <EOS>
  unsigned int Length() const { return words.size(); }

  unsigned int number;
  std::vector<Word> words;
};

struct Input {
  Input() {}

  const Word &GetWord(unsigned int engine, unsigned int offset) const {
    return engines[engine].words[offset];
  }

  const Word &GetWord(const Location &l) const {
    return engines[l.engine].words[l.offset];
  }

  unsigned int NumEngines() const {
    return engines.size();
  }

  void SetupEngines(unsigned int count) {
    engines.resize(count);
    for (unsigned int i = 0; i < engines.size(); ++i) {
      engines[i].number = i;
    }
  }

  std::vector<Engine> engines;
};

} // namespace input

#endif
