#ifndef _MEMT_Feature_LM_Config_h
#define _MEMT_Feature_LM_Config_h

#include "MEMT/Feature/Base/Sign.hh"

#include "util/numbers.hh"

#include <vector>

namespace feature {
namespace lm {

struct Config {
  // Include counts for each n-gram length?
  bool by_length;
  std::vector<unsigned char> orders;
  size_t count;
  size_t FeatureCount() const { return count; }
  void WeightHint(base::WeightSign *out) const;
};

} // namespace lm
} // namespace feature

#endif // _MEMT_Feature_LM_Config_h
