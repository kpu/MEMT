#include "MEMT/Input/Input.hh"

#include <unicode/uchar.h>
#include <unicode/unistr.h>
#include <unicode/utf8.h>

#include <stdexcept>

namespace input {

void CollectVotes(const std::string &word, bool skip_first, std::vector<size_t> &uppercase, std::vector<size_t> &lowercase) {
  int32_t offset = 0;
  int32_t length = static_cast<uint32_t>(word.size());
  for (unsigned int i = 0; offset < length; ++i) {
    UChar32 character;
    U8_NEXT(word.data(), offset, length, character);
    // TODO: throw a better exception
    if (character < 0) throw std::runtime_error("Not UTF8");
    if (uppercase.size() <= i) {
      uppercase.resize(i + 1);
      lowercase.resize(i + 1);
    }
    if (u_isUUppercase(character)) {
      ++uppercase[i];
    }
    if (u_isULowercase(character)) {
      ++lowercase[i];
    }
  }
}

void ApplyVotes(std::string &word, std::vector<size_t> &uppercase, std::vector<size_t> &lowercase) {
  UnicodeString copied(UnicodeString::fromUTF8(word));
  for (int32_t i = 0; (i < copied.length()) && (i < static_cast<int32_t>(uppercase.size())); ++i) {
    copied.setCharAt(i, (uppercase[i] > lowercase[i]) ? u_toupper(copied.charAt(i)) : u_tolower(copied.charAt(i)));
  }
  word.clear();
  copied.toUTF8String(word);
}

void ApplyCapitalization(Input &input) {
  for (std::vector<Engine>::iterator e = input.engines.begin(); e != input.engines.end(); ++e) {
    for (std::vector<Word>::iterator w = e->words.begin() + 1; w < e->words.end() - 1; ++w) {
      std::vector<size_t> uppercase, lowercase;
      for (unsigned int engine = 0; engine < input.engines.size(); ++engine) {
        const AlignmentInfo &info = w->alignments.Ask(engine);
        if (info.type & (AL_EXACT | AL_SELF)) {
          CollectVotes(input.GetWord(engine, info.offset).text.Original(), info.offset == 1, uppercase, lowercase);
        }
      }
      ApplyVotes(w->text.MutableOriginalForCase(), uppercase, lowercase);
    }
  }
}

} // namespace input
