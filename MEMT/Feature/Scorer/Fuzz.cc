#include "MEMT/Feature/Scorer/Fuzz.hh"

#include "util/numbers.hh"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#include <vector>

namespace feature {
namespace scorer {

void Fuzz::Apply(const std::vector<LinearScore> &in_weights, std::vector<LinearScore> &out_weights) {
  if (ratio_ <= 0.0) {
    out_weights = in_weights;
    return;
  }
  boost::uniform_real<LinearScore> dist(1.0 - ratio_, 1.0 + ratio_);
  boost::variate_generator<boost::mt19937 &, boost::uniform_real<LinearScore> > sample(rng_, dist);

  out_weights.clear();
  for (std::vector<LinearScore>::const_iterator i = in_weights.begin(); i != in_weights.end(); ++i) {
    out_weights.push_back(*i * sample());
  }
}

} // namespace scorer
} // namespace feature
