#ifndef _MEMT_Feature_Scorer_Fuzz_h
#define _MEMT_Feature_Scorer_Fuzz_h

// Randomly modifies weights based on their existing values.  This is used in
// a basic simulated annealing for tuning.

#include "util/numbers.hh"

#include <boost/scoped_ptr.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <vector>

namespace feature {
namespace scorer {

class Fuzz {
  public:
    void Reset(LinearScore ratio) { ratio_ = ratio; }

    void Apply(const std::vector<LinearScore> &in_weights, std::vector<LinearScore> &out_weights);

  private:
    LinearScore ratio_;

    boost::mt19937 rng_;
};

} // namespace scorer
} // namespace feature

#endif // _MEMT_Feature_Scorer_Fuzz_h
