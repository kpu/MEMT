#include "MEMT/Decoder/Score.hh"

namespace decoder {

std::ostream &operator<<(std::ostream &s, const Score &score) {
  s << "overall=" << score.Overall().Log();
  s << ", features = \"";
  for (std::vector<LogScore>::const_iterator i = score.Features().begin(); i != score.Features().end(); ++i) {
    if (i != score.Features().begin()) s << ' ';
    s << i->Log();
  }
  s << '"';
  return s;
}

} // namespace decoder
