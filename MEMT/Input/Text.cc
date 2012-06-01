#include "MEMT/Input/Text.hh"
#include "util/utf8.hh"
#include "util/murmur_hash.hh"

#include <string>

using namespace std;

namespace input {

void WordText::ResetBOS() {
  original_ = "<s>";
  canonical_ = "<s>";
  is_punctuation_ = false;
  is_end_ = false;
  canonical_hash_ = util::MurmurHashNative(canonical_.c_str(), canonical_.size());
}

void WordText::ResetEOS() {
  original_ = "</s>";
  canonical_ = "</s>";
  is_punctuation_ = false;
  is_end_ = true;
  canonical_hash_ = util::MurmurHashNative(canonical_.c_str(), canonical_.size());
}

void WordText::RereadOriginal(bool lowercase_canonical) {
  if (lowercase_canonical) {
    utf8::ToLower(original_, canonical_);
  } else {
    canonical_ = original_;
  }
  is_punctuation_ = utf8::IsPunctuation(canonical_);
  is_end_ = false;
  canonical_hash_ = util::MurmurHashNative(canonical_.c_str(), canonical_.size());
}

} // namespace input
