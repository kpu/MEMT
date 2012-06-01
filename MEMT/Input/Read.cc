#include "MEMT/Input/Config.hh"
#include "MEMT/Input/Input.hh"
#include "util/tokenize_piece.hh"

#include <string>

namespace input {

void ReadEngine(const Config &config, const std::string &line, unsigned int num_engines, Engine &engine) {
  std::vector<Word> &words = engine.words;
  if (words.empty()) { words.resize(2); }
  words[0].ResetBOS(num_engines, 0);

  unsigned int offset = 1;
  for (util::PieceIterator<' '> original(line); original; ++original, ++offset) {
    if (offset + 1 > words.size()) {
      words.resize(offset + 1);
    }
    words[offset].ResetWord(num_engines, offset, config.lowercase_before_lm, *original);
  }

  words.resize(offset + 1);
  words[offset].ResetEOS(num_engines, offset);
}

void ReadAllEngines(const Config &config, std::istream &in, Input &input) {
  std::string line;
  for (std::vector<Engine>::iterator e = input.engines.begin(); e != input.engines.end(); ++e) {
    getline(in, line);
    ReadEngine(config, line, input.engines.size(), *e);
  }
}

void AddSelfAlignments(Input &input) {
  for (std::vector<Engine>::iterator e = input.engines.begin(); e != input.engines.end(); ++e) {
    for (std::vector<Word>::iterator w = e->words.begin(); w != e->words.end(); ++w) {
      w->alignments.Add(e->number, w - e->words.begin(), AL_SELF);
    }
  }
}

void AddBoundaryAlignments(Input &input) {
  for (unsigned int e1 = 0; e1 < input.engines.size(); ++e1) {
    Engine &engine1 = input.engines[e1];
    std::vector<Word> &words1 = engine1.words;
    for (unsigned int e2 = 0; e2 < e1; ++e2) {
      Engine &engine2 = input.engines[e2];
      std::vector<Word> &words2 = engine2.words;
      words1[0].alignments.Add(e2, 0, AL_BOUNDARY);
      words2[0].alignments.Add(e1, 0, AL_BOUNDARY);
      words1.back().alignments.Add(e2, words2.size() - 1, AL_BOUNDARY);
      words2.back().alignments.Add(e1, words1.size() - 1, AL_BOUNDARY);
    }
  }
}

} // namespace input
