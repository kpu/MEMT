#include "MEMT/Strategy/Phrase/Phrase.hh"

#include "MEMT/Input/Input.hh"
#include "MEMT/Strategy/Phrase/Aligned.hh"
#include "MEMT/Strategy/Phrase/Punctuation.hh"

namespace strategy {
namespace phrase {

// This could be faster with a list of phrases, but I'm not sure it's worth it. 
void ComputeBlocks(System &system) {
  for (unsigned int offset = 0; offset < system.size(); ++offset) {
    const std::map<unsigned int, Type> &ends = system[offset].Ends();
    for (std::map<unsigned int, Type>::const_iterator p = ends.begin(); p != ends.end(); ++p) {
      if (!(p->second & (PHRASE_PUNCTUATION | PHRASE_SOURCE_CHUNK))) continue;
      PhraseRange blockage;
      blockage.start = offset;
      blockage.end = p->first;
      for (unsigned int j = offset; j <= p->first; ++j) {
        system[j].AddBlock(blockage);
      }
    }
  }
}

std::ostream &operator<<(std::ostream &str, const Word &phrases) {
  for (std::map<unsigned int, Type>::const_iterator
      i = phrases.Ends().begin(); i != phrases.Ends().end(); ++i) {
    str << "Phrase end=" << i->first << " type=";
    TypeOut(str, i->second) << '\n';
  }
  str << "Block [" << phrases.Block().start << "," << phrases.Block().end << "]";
  return str;
}

void System::Reset(Type types, const input::Engine &engine) {
  words_.resize(engine.words.size());
  for (size_t i = 0; i < engine.words.size(); ++i) {
    words_[i].Reset(i);
  }
  if (types & PHRASE_PUNCTUATION) DetectPunctuation(engine, *this);
  if (types & PHRASE_ALIGNMENT) DetectAligned(engine, *this);
  ComputeBlocks(*this);
}

void Sentence::Reset(Type types, const input::Input &input) {
  systems_.resize(input.engines.size());
  for (size_t i = 0; i < input.engines.size(); ++i) {
    systems_[i].Reset(types, input.engines[i]);
  }
}

} // namespace phrase
} // namespace strategy
