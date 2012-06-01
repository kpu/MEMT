#ifndef _MEMT_Input_Config_h
#define _MEMT_Input_Config_h

#include "util/numbers.hh"

#include <ostream>
#include <string>
#include <vector>

namespace input {

struct Config {
  bool transitive;

  // TODO: this really belongs with the LM feature.
  bool lowercase_before_lm;
};

} // namespace input

#endif
