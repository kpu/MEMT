#ifndef _MEMT_Alignment_NBest_h
#define _MEMT_Alignment_NBest_h

#include "util/exception.hh"
#include "util/file_piece.hh"

#include <limits>

namespace input {

class NBestException : public util::Exception {
  public:
    NBestException() throw();
    ~NBestException() throw();
};

class NBestReader {
  public:
    explicit NBestReader(const char *file);

    bool ReadEntry(unsigned int segment, StringPiece &out);

    bool Ended() const {
      return next_segment_ == std::numeric_limits<unsigned int>::max();
    }

  private:
    void ReadSegmentID();

    util::FilePiece file_;

    unsigned int next_segment_;
};

} // namespace input

#endif // _MEMT_Alignment_NBest_h
