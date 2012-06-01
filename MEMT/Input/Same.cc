#error "Not finished"
#include "MEMT/Input/Same.hh"
#include "MEMT/Input/Location.hh"

#include <set>

namespace input {

bool AlignmentsSameUpToType(const WordAlignments &left, const WordAlignments &right) {
  const WordAlignments::Map &lmap = left.Get();
  const WordAlignments::Map &rmap = right.Get();
  for (WordAlignments::Map liter = lmap.begin(), riter = rmap.begin(); ; ++liter, ++riter) {
    if (liter == liter.end()) {
      if (riter == riter.end()) return true;
    } else {
      return false;
    }
    if (riter == riter.end()) return false;
    if (liter->first != riter->first) return false;
    if (liter->second.offset != riter->second.offset) return false;
  }
}

void FindSame(Input &text) {
  std::set<Location> todo;
  for (unsigned int e = 0; e < text.engines.size(); ++e) {
    for (unsigned int o = 0; o < text.engines[e].words.size(); ++o) {
      todo.insert(todo.end(), Location(e, o));
    }
  }
  while (!todo.empty()) {
    const Location &root = *todo.begin();
    const Word &word = text.GetWord(root);
    const WordAlignments::Map &aligns = word.alignments.Get();
    for (WordAlignments::Map::const_iterator i = aligns.begin(); i != aligns.end(); ++i) {
      if (!(i->second.type & AL_EXACT)) continue;
      const Word &other = text.GetWord(i->first, i->second.offset);
      if (!AlignmentsSameUpToType(left, 
    }

  }
}

}  // namespace input
