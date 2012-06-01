#ifndef _MEMT_Decoder_Config_h
#define _MEMT_Decoder_Config_h

#include "MEMT/Strategy/Phrase/Type.hh"
#include "MEMT/Strategy/Horizon/Config.hh"

#include "util/debug.hh"
#include "util/numbers.hh"

#include <ostream>
#include <vector>

namespace decoder {

struct Config {
  unsigned int internal_beam_size, end_beam_size;
  bool length_normalize;
};

std::ostream &operator<<(std::ostream &str, const Config &config);

} // namespace decoder

#endif
