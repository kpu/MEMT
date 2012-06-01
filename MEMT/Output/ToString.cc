#include "MEMT/Output/ToString.hh"

#include "MEMT/Decoder/Completed.hh"
#include "MEMT/Input/Input.hh"
#include "MEMT/Input/Text.hh"
#include "MEMT/Output/Config.hh"
#include "util/utf8.hh"

#include <unicode/uchar.h>
#include <unicode/utf8.h>

#include <stdexcept>

namespace output {

void InitialCap(std::string &word) {
  int32_t offset = 0;
  int32_t length = static_cast<uint32_t>(word.size());
  UChar32 character;
  U8_NEXT(word.data(), offset, length, character);
  if (character < 0) throw std::runtime_error("not utf8");
  character = u_toupper(character);
  uint8_t firstutf8[4];
  int32_t i = 0;
  UBool error = false;
  U8_APPEND(firstutf8, i, 4, character, error);
  if (error) throw std::runtime_error("Applying case");
  word.replace(0, offset, reinterpret_cast<const char*>(firstutf8), i);
}

void CompletedHypothesisString(const Config &config, const decoder::CompletedHypothesis &hyp, const input::Input &text, std::string *out) {
  out->clear();
  const std::vector<input::Location> &words = hyp.Words();
  if (words.size() < 3) return;
  std::vector<input::Location>::const_iterator ending = words.end() - 1;
  std::string tmp;
  for (std::vector<input::Location>::const_iterator 
      i = words.begin() + 1; i != ending; ++i) {
    const input::WordText &t = text.engines[i->engine].words[i->offset].text;
    if (config.lowercase) {
      utf8::ToLower(t.Original(), tmp);
      *out += tmp;
    } else if (config.initial_cap && (i == words.begin() + 1)) {
      *out = t.Original();
      InitialCap(*out);
    } else {
      *out += t.Original();
    }
    out->push_back(' ');
  }
}

} // namespace output
