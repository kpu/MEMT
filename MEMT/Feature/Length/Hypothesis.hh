#ifndef _MEMT_Feature_Length_Hypothesis_h
#define _MEMT_Feature_Length_Hypothesis_h

namespace feature {
namespace length {

struct Hypothesis {};

bool operator==(const Hypothesis left, const Hypothesis right) {
  return true;
}

size_t hash_value(const Hypothesis value) {
  // Mashing on keyboard.
  return 415648974;
}

} // namespace length
} // namespace feature

#endif // _MEMT_Feature_Length_Hypothesis_h
