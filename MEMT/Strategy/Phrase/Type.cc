#include "MEMT/Strategy/Phrase/Type.hh"

namespace strategy {
namespace phrase {

std::ostream &TypeOut(std::ostream &str, const Type type) {
  if (type & PHRASE_PUNCTUATION) str << "punctuation ";
  if (type & PHRASE_ALIGNMENT) str << "alignment ";
  if (type & PHRASE_SOURCE_CHUNK) str << "source_chunk ";
  return str;
}

} // namespace phrase
} // namespace strategy
