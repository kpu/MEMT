#ifndef _MEMT_Input_Text_h
#define _MEMT_Input_Text_h

#include "util/string_piece.hh"

#include <string>

#include <inttypes.h>

namespace input {

class WordText {
  public:
    WordText() {}

    // Sets everything but vocab_index_, which must be set later.
    void Reset(bool lowercase_canonical, const StringPiece &original) {
      original_.assign(original.data(), original.size());
      RereadOriginal(lowercase_canonical);
    }

    void ResetBOS();

    void ResetEOS();

    const std::string &Original() const { return original_; }
    const std::string &Canonical() const { return canonical_; }
    bool IsPunctuation() const { return is_punctuation_; }

    bool IsEnd() const { return is_end_; }

    std::string &MutableOriginalForCase() { return original_; }

    uint64_t CanonicalHash() const { return canonical_hash_; }

  private:
    void RereadOriginal(bool lowercase_canonical);

    // UTF8 input string.
    std::string original_;
    // UTF8 lowercased.
    std::string canonical_;
    bool is_punctuation_;
    bool is_end_;

    uint64_t canonical_hash_;
};

}

#endif
