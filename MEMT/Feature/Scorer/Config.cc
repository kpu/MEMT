#include "MEMT/Feature/Scorer/Config.hh"

#include <boost/fusion/algorithm.hpp>

namespace feature {
namespace scorer {

namespace {
struct GatherWeightHints {
  typedef base::WeightSign *result_type;

  template <class Feature> result_type operator()(const result_type previous, const Feature &feature) const {
    feature.WeightHint(previous);
    return previous + feature.FeatureCount();
  }
};

struct FeatureCountFold {
  typedef unsigned result_type;
  template <class Feature> result_type operator()(const unsigned previous, const Feature &feature) const {
    return previous + feature.FeatureCount();
  }
};
} // namespace

void Config::WeightHint(base::WeightSign *out) const {
  fold(features, out, GatherWeightHints());
}

unsigned Config::FeatureCount() const {
  return fold(features, 0, FeatureCountFold());
}

} // namespace scorer
} // namespace feature
