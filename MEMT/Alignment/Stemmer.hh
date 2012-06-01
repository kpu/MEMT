#ifndef _MEMT_Alignment_Stemmer_h
#define _MEMT_Alignment_Stemmer_h

#include "util/string_piece.hh"

struct sb_stemmer;

namespace input {

class SnowballWrap {
  public:
    explicit SnowballWrap(const char *language);

    ~SnowballWrap();

    // The returned StringPiece is invalidated after each call.  Sadly non-const.  
    StringPiece Stem(const StringPiece &word);

  private:
    sb_stemmer *stemmer_;
};

} // namespace input

#endif // _MEMT_Alignment_Stemmer_h
