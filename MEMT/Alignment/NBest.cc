#include "MEMT/Alignment/NBest.hh"

#include <assert.h>
#include <ctype.h>

namespace input {

NBestException::NBestException() throw() {}
NBestException::~NBestException() throw() {}

NBestReader::NBestReader(const char *file) : file_(file) {
  ReadSegmentID();
}

namespace {
void SkipSpaces(StringPiece &str) {
  while (str.size() && isspace(*str.data()))
    str.set(str.data() + 1, str.size() - 1);
}
} // namespace

bool NBestReader::ReadEntry(unsigned int segment, StringPiece &out) {
  if (segment < next_segment_) return false;
  assert(segment == next_segment_);
  out = file_.ReadLine();
  SkipSpaces(out);
  if (out.size() < 3 || out.substr(0, 3) != "|||")
    UTIL_THROW(NBestException, "Three pipes missing in " << out);
  SkipSpaces(out);
  // Argh no portable strnstr or find
  for (const char *i = out.data(); i < out.data() + out.size() - 2; ++i) {
    if (*i == '|' && *(i+1) == '|' && *(i+2) == '|') {
      out.set(out.data(), i - out.data());
      break;
    }
  }
  ReadSegmentID();
  return true;
}

void NBestReader::ReadSegmentID() {
  try {
    next_segment_ = file_.ReadULong();
  } catch(const util::EndOfFileException &e) {
    next_segment_ = std::numeric_limits<unsigned int>::max();
  }
}

} // namespace input
