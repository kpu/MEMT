#include "MEMT/Decoder/Config.hh"

#include "MEMT/Strategy/Phrase/Type.hh"

namespace decoder {

std::ostream &operator<<(std::ostream &str, const Config &config) {
  return str
    << "beam_size = " << config.internal_beam_size << '\n'
    << "output.nbest = " << config.end_beam_size << '\n'
    << "length_normalize = " << config.length_normalize << '\n';
}

} // namespace decoder
