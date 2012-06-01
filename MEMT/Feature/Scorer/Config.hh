#ifndef _MEMT_Feature_Scorer_Config_h
#define _MEMT_Feature_Scorer_Config_h

#include "MEMT/Feature/Base/Sign.hh"
#include "MEMT/Feature/Length/Config.hh"
#include "MEMT/Feature/LM/Config.hh"
#include "MEMT/Feature/Verbatim/Config.hh"

#include <boost/fusion/container/vector.hpp>

namespace feature {
namespace scorer {

struct Config {
  // TODO: this should be tied to main's idea of the features.
  typedef boost::fusion::vector<lm::Config, verbatim::Config, verbatim::Config, length::Config> Features;
  Features features;
  std::vector<LinearScore> weights;
  LinearScore fuzz_ratio;

  unsigned FeatureCount() const;

  void WeightHint(base::WeightSign *out) const;
};

} // namespace scorer
} // namespace feature

#endif // _MEMT_Feature_Scorer_Config_h
