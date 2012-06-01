#ifndef _MEMT_Feature_Verbatim_Config_h
#define _MEMT_Feature_Verbatim_Config_h

#include "MEMT/Feature/Base/Sign.hh"
#include "MEMT/Input/Alignment.hh"

#include "util/numbers.hh"

#include <cstddef>
#include <vector>

#include <assert.h>

namespace feature {
namespace verbatim {

struct Config {
  std::size_t num_systems;
  // ngram length for individual (per-system) scores
  std::size_t individual;
  // max ngram length for equal weight scores.  This should be >= individual.
  std::size_t collective;

  // Mask of alignments that count as supporting.
  input::AlignType mask;

  size_t FeatureCount() const {
    assert(collective >= individual);
    assert(num_systems != 0);
    return num_systems * individual + collective - individual;
  }

  void WeightHint(base::WeightSign *out) const {
    base::WeightSign *end = out + FeatureCount();
    for (; out != end; ++out) *out = base::POSITIVE_WEIGHT;
  }
};

} // namespace verbatim
} // namespace feature

#endif // _MEMT_Feature_Verbatim_Config_h
