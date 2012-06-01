#ifndef _MEMT_Output_NBest_h
#define _MEMT_Output_NBest_h

#include "MEMT/Decoder/Completed.hh"

#include <ostream>
#include <vector>

namespace input { class Input; }

namespace output {

class Config;

void NBest(std::ostream &out, const Config &config, const std::vector<decoder::CompletedHypothesis> &nbest, const input::Input &text, unsigned int sent_id);

} // namespace output

#endif
