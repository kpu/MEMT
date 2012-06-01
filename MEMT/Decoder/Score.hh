#ifndef _MEMT_Decoder_Score_h
#define _MEMT_Decoder_Score_h
// Scores for the various features and their overall combination.

#include "util/numbers.hh"

#include <ostream>
#include <vector>

namespace decoder {

// Pretty much a struct for now.
class Score {
  public:
    const LogScore &Overall() const { return overall_; }
    LogScore &MutableOverall() { return overall_; }

    const std::vector<LogScore> &Features() const { return features_; }
    std::vector<LogScore> &MutableFeatures() { return features_; }

    // The length feature should not yet be applied.  
    void NormalizeByLength(size_t length) {
      LinearScore exponent(1.0 / static_cast<LinearScore>(length));
      overall_.PowEquals(exponent);
      for (std::vector<LogScore>::iterator i(features_.begin()); i != features_.end(); ++i) {
        i->PowEquals(exponent);
      }
    }

  private:
    LogScore overall_;

    std::vector<LogScore> features_;
};

inline bool operator<(const Score &left, const Score &right) {
  return left.Overall() < right.Overall();
}

inline bool operator>(const Score &left, const Score &right) {
  return left.Overall() > right.Overall();
}

inline Score &operator/=(Score &to, const Score &by) {
  to.MutableOverall() /= by.Overall();
  assert(to.Features().size() == by.Features().size());
  std::vector<LogScore>::iterator ti(to.MutableFeatures().begin());
  std::vector<LogScore>::const_iterator bi(by.Features().begin());
  for (; bi != by.Features().end(); ++ti, ++bi) {
    *ti /= *bi;
  }
  return to;
}

inline Score &operator*=(Score &to, const Score &by) {
  to.MutableOverall() *= by.Overall();
  assert(to.Features().size() == by.Features().size());
  std::vector<LogScore>::iterator ti(to.MutableFeatures().begin());
  std::vector<LogScore>::const_iterator bi(by.Features().begin());
  for (; bi != by.Features().end(); ++ti, ++bi) {
    *ti *= *bi;
  }
  return to;
}

std::ostream &operator<<(std::ostream &s, const Score &score);

} // namespace decoder

#endif
