#include "MEMT/Alignment/Stemmer.hh"
#include "util/exception.hh"

#include "MEMT/Alignment/libstemmer_c/include/libstemmer.h"

namespace input {

SnowballWrap::SnowballWrap(const char *language) : stemmer_(sb_stemmer_new(language, NULL)) {
  if (!stemmer_) UTIL_THROW(util::Exception, "Failed to create stemmer for " << language);
}

SnowballWrap::~SnowballWrap() {
  if (stemmer_) sb_stemmer_delete(stemmer_);
}

StringPiece SnowballWrap::Stem(const StringPiece &word) {
  // Snowball likes const unsigned char.  StringPiece likes const char.  
  const char *data = reinterpret_cast<const char*>(sb_stemmer_stem(stemmer_, reinterpret_cast<const unsigned char*>(word.data()), word.size()));
  if (!data) UTIL_THROW(util::Exception, "Stemming " << word << " returned NULL.");
  return StringPiece(data, sb_stemmer_length(stemmer_));
}

} // namespace input
