#ifndef _MEMT_Strategy_Phrase_Type_h
#define _MEMT_Strategy_Phrase_Type_h

#include <ostream>

namespace strategy {
namespace phrase {

typedef unsigned int Type;

const Type PHRASE_PUNCTUATION = 1 << 0;
const Type PHRASE_ALIGNMENT = 1 << 1;
const Type PHRASE_SOURCE_CHUNK = 1 << 2;

std::ostream &TypeOut(std::ostream &str, const Type type);

} // namespace phrase
} // namespace strategy

#endif // _MEMT_Strategy_Phrase_Type_h
