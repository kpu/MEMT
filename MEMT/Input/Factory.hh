#ifndef _MEMT_Input_Factory_h
#define _MEMT_Input_Factory_h

#include "MEMT/Input/Capitalization.hh"
#include "MEMT/Input/Config.hh"
#include "MEMT/Input/Transitive.hh"

namespace input {

class Input;

void ProcessAligned(
    const Config &config,
    Input &input) {
  if (config.transitive) MakeAlignmentsTransitive(input);
  ApplyCapitalization(input);
}

}  // namespace input

#endif
