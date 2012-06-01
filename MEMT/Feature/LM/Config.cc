#include "MEMT/Feature/LM/Config.hh"

namespace feature {
namespace lm {

void Config::WeightHint(base::WeightSign *out) const {
  for (std::vector<unsigned char>::const_iterator l = orders.begin(); l != orders.end(); ++l) {
    // probability
    *(out++) = base::POSITIVE_WEIGHT;
    // <unk>
    *(out++) = base::ANY_WEIGHT;
    if (by_length) {
      for (unsigned char i = 1; i < *l; ++i)
        // length count
        *(out++) = base::ANY_WEIGHT;
    }
  }
}

} // namespace lm
} // namespace feature
