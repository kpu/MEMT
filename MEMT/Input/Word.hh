#ifndef _MEMT_Input_Word_h
#define _MEMT_Input_Word_h

#include "MEMT/Input/Alignment.hh"
#include "MEMT/Input/Text.hh"

#include "util/numbers.hh"
#include "util/string_piece.hh"

namespace input {

  // Precomputed information about a word.
  // This class is scheduled for partitioning into alignment and precompute pieces.
  struct Word {
    Word() {}

    void ResetBOS(unsigned int engines, unsigned int offset) {
      text.ResetBOS();
      alignments.Reset(engines);
    }

    void ResetEOS(unsigned int engines, unsigned int offset) {
      text.ResetEOS();
      alignments.Reset(engines);
    }

    void ResetWord(unsigned int engines, unsigned int offset, bool lowercase_canonical, const StringPiece &word) {
      alignments.Reset(engines);
      text.Reset(lowercase_canonical, word);
    }

    // Actual input.
    WordText text;
    WordAlignments alignments;
  };

}  // namespace

#endif
