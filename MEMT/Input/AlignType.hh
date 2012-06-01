#ifndef _MEMT_Input_AlignType_h
#define _MEMT_Input_AlignType_h

#include <string>

#include "util/string_piece.hh"

namespace input {
typedef unsigned int AlignType;
const AlignType AL_UNKNOWN       = 1 << 0;
const AlignType AL_EXACT         = 1 << 1;
const AlignType AL_SNOWBALL_STEM = 1 << 2;
const AlignType AL_WN_STEM       = 1 << 3;
const AlignType AL_WN_SYNONYMY   = 1 << 4;
const AlignType AL_PARAPHRASE    = 1 << 5;
const AlignType AL_ARTIFICIAL    = 1 << 6;
// Words always have at least one alignment: themselves.
const AlignType AL_SELF          = 1 << 7;
// Alignments are made transitive if make transitive is set unless there is a conflict.  
const AlignType AL_TRANSITIVE    = 1 << 8;
// Sentence boundaries are aligned using this type.
const AlignType AL_BOUNDARY      = 1 << 9;

const AlignType AL_COUNT = 10;

extern const char *kAlignTypeNames[AL_COUNT];

class NotAlignmentTypeName : public std::exception {
  public:
    explicit NotAlignmentTypeName(const StringPiece &name) throw();

    ~NotAlignmentTypeName() throw() {}

    const char *what() const throw() { return what_.c_str(); }

  private:
    std::string what_;
};

AlignType TypeFromName(const StringPiece &name);

template <class Iterator> AlignType TypeFromNames(const Iterator &begin, const Iterator &end) {
  AlignType ret = 0;
  for (Iterator i(begin); i != end; ++i) {
    ret |= TypeFromName(*i);
  }
  return ret;
}

} // namespace input

#endif // _MEMT_Input_AlignType_h
