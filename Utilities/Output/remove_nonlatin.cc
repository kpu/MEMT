#include "util/tokenize_piece.hh"

#include <unicode/uscript.h>
#include <unicode/utf8.h>

#include <string>
#include <iostream>

bool RemoveWord(const StringPiece &str) {
  int32_t size_as_int32 = static_cast<uint32_t>(str.size());
  UChar32 character = -1;
  for (int32_t offset = 0; offset < size_as_int32;) {
    U8_NEXT(str.data(), offset, size_as_int32, character);
    if (character < 0) {
      std::cerr << "Bad UTF8 " << str.data()[offset] << " in " << str << std::endl;
      return true;
    }
    UErrorCode err = UErrorCode();
    UScriptCode code = uscript_getScript(character, &err);
    if (err) {
      std::cerr << u_errorName(err) << std::endl;
      exit(1);
    }
    if (code == USCRIPT_LATIN || code == USCRIPT_COMMON) {
      return false;
    }
  }
  return true;
}

int main() {
  std::string line;
  while (std::getline(std::cin, line)) {
    bool rest = false;
    for (util::PieceIterator<' '> i(line); i; ++i) {
      if (!RemoveWord(*i)) {
        if (rest) std::cout << ' ';
        rest = true;
        std::cout << *i;
      }
    }
    std::cout << '\n';
  }
}
