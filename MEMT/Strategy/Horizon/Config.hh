#ifndef _MEMT_Strategy_Horizon_Config_h
#define _MEMT_Strategy_Horizon_Config_h

#include "util/numbers.hh"

#include <vector>

// Horizon configuration is part of Input configuration and passed to the
// decoder.  This is a separate file because it's also part of Input.

namespace strategy {
namespace horizon {

struct Config {
  typedef enum {HORIZON_LENGTH, HORIZON_ALIGNMENT} Method;
  Method method;

  unsigned int radius;

  // Weights for stay_threshold.  Defaults to uniform.
  std::vector<LinearScore> stay_weights;
  // Applies only for method = HORIZON_ALIGNMENT.
  LinearScore stay_threshold;
};

} // namespace horizon
} // namespace strategy

#endif // _MEMT_Strategy_Horizon_Config_h
