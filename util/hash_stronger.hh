/* Boost's hash is incredibly weak: char's hash is identity.  I've seen several
 * collisions as a result.  These 64-bit hash functions are stronger and
 * intended for use where collisions are not checked, but incur a small cost.  
 * For example, the language model filter will pass more n-grams than necessary
 * in the event of a collision.  In these cases, a slightly stronger hash
 * function is useful.  
 */
#include "util/string_piece.hh"

namespace util {

// 64-bit extension of boost's hash_combine with arbitrary number prepended.
inline void CombineHash(uint64_t &seed, uint64_t with) {
  seed ^= with + 0x8fd2736e9e3779b9ULL + (seed<<6) + (seed>>2);
}

// This spreads a char's hash over all 64 bits.  
inline uint64_t StringHash(const StringPiece &str) {
  uint64_t ret = 0;
  for (const char *i = str.data(); i != str.data() + str.size(); ++i) {
    char val = *i;
    CombineHash(ret, static_cast<uint64_t>(val) * 0x478387ef381de5e3ULL);
  }
  return ret;
}

} // namespace util
