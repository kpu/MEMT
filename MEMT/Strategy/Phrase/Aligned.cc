#include "MEMT/Strategy/Phrase/Aligned.hh"

#include "MEMT/Input/Input.hh"
#include "MEMT/Input/Alignment.hh"
#include "MEMT/Strategy/Phrase/Phrase.hh"

#include <map>

namespace strategy {
namespace phrase {

namespace {

void SeedParallel(const input::WordAlignments &from, std::map<unsigned int, unsigned int> &parallel) {
  for (input::WordAlignments::const_iterator i = from.begin(); i != from.end(); ++i) {
    if (i->IsConnection())
      parallel.insert(parallel.end(), std::make_pair(i - from.begin(), i->offset));
  }
}

} // namespace

// Alignment phrases happen when two systems align consecutively.  We take the 
// longest such phrase.  This was inherited.  I don't think this makes much
// sense and neither does a WMT reviewer.
void DetectAligned(const input::Engine &engine, System &system) {
  if (engine.words.size() < 2) return;

  // Map from consecutively aligning system to offset.
  std::map<unsigned int, unsigned int> parallel;

  // Start with the first non-BOS word.
  SeedParallel(engine.words[1].alignments, parallel);

  unsigned int start = 1;
  unsigned int word;
  for (word = 2; word < engine.words.size(); ++word) {
    const input::WordAlignments &align = engine.words[word].alignments;
    // Cut parrallel down to those alignments continuing the phrase.
    for (std::map<unsigned int, unsigned int>::iterator i = parallel.begin(); i != parallel.end();) {
      const input::AlignmentInfo &info = align.Ask(i->first);
      if (info.IsNone() || (info.offset != i->second + 1)) {
        parallel.erase(i++);
      } else {
        ++i->second;
        ++i;
      }
    }
    // If the only remaining parallel phrase is itself
    if (parallel.size() <= 1) {
      if (start != word - 1) {
        system[start].AddEnd(word - 1, PHRASE_ALIGNMENT);
      }
      start = word;
      SeedParallel(align, parallel);
    }
  }
  // TODO(kheafiel): look at EOS condition in more detail here.
  if (start + 1 < word) {
    system[start].AddEnd(word - 1, PHRASE_ALIGNMENT);
  }
}

}  // namespace phrase
}  // namespace strategy
