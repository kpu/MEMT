#ifndef _MEMT_Feature_Length_Config_h
#define _MEMT_Feature_Length_Config_h

#include "MEMT/Feature/Base/Sign.hh"

namespace feature {
namespace length {

struct Config {
  unsigned FeatureCount() const { return 1; }
  void WeightHint(base::WeightSign *out) const { *out = base::ANY_WEIGHT; }
};

} // namespace length
} // namespace feature

#endif // _MEMT_Feature_Length_Config_h
