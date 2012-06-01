#ifndef _MEMT_Decoder_Completed_hh
#define _MEMT_Decoder_Completed_hh

#include "MEMT/Decoder/Score.hh"
#include "MEMT/Input/Location.hh"

#include <vector>

namespace decoder {

// Final hypothesis produced by the decoder.  This is the only one that should be used outside.
class CompletedHypothesis {
  public:
    CompletedHypothesis() {}

    void Reset(const Score &score, const std::vector<LogScore> &end_features) {
      score_ = score; 
      end_features_ = end_features;
      words_.clear();
    }

    void AppendWord(const input::Location &source) {
      words_.push_back(source);
    }

    const std::vector<input::Location> &Words() const { return words_; }

    // If length_normalize is set, this is normalized.
    const Score &GetScore() const { return score_; }

    const std::vector<LogScore> &EndFeatures() const { return end_features_; }

  private:
    std::vector<input::Location> words_;

    Score score_;

    std::vector<LogScore> end_features_;
};

} // namespace decoder

#endif
