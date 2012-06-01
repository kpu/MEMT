#include "MEMT/Input/Alignment.hh"

using namespace std;

namespace input {

const AlignType AL_IGNORE_SCORE = AL_ARTIFICIAL | AL_TRANSITIVE;

inline bool ListenToScore(AlignType type) {
  return type & ~AL_IGNORE_SCORE;
}

void WordAlignments::Add(unsigned int engine, unsigned int offset, AlignType type) {
  assert(alignments_[engine].IsNone() || (alignments_[engine].offset == offset));
  alignments_[engine].type |= type;
  alignments_[engine].offset = offset;
}

}  // namespace input
