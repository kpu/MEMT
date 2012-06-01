#ifndef _MEMT_Output_ToString_h
#define _MEMT_Output_ToString_h

#include <string>

namespace decoder { class CompletedHypothesis; }

namespace input { class Input; }

namespace output {
class Config;
void CompletedHypothesisString(const Config &config, const decoder::CompletedHypothesis &hyp, const input::Input &text, std::string *out);
} // namespace output

#endif
