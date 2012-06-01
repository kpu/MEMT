#ifndef _MEMT_Strategy_Phrase_Hypothesis_h
#define _MEMT_Strategy_Phrase_Hypothesis_h

#include "MEMT/Input/Word.hh"
#include "MEMT/Strategy/Horizon/Hypothesis.hh"
#include "MEMT/Strategy/Phrase/Phrase.hh"

#include <boost/functional/hash.hpp>

#include <limits.h>

namespace strategy {
namespace phrase {

// Manage phrase-locking of a hypothesis.
class Hypothesis {
  public:
    Hypothesis() {}

    void InitializeRoot() {
      engine_ = UINT_MAX;
      end_ = 0;
    }

    bool InPhrase() const { return engine_ != UINT_MAX; }

    unsigned int Engine() const { return engine_; }

    unsigned int End() const { return end_; }

    /* This is inline because it will return on the first if statement a lot.
     * As a precondition, word:
     *   Has already been used in new_coverage
     *   Came from advancing_engine
     *   Has offset added_word_offset in the advancing engine
     * This sets up phrase constraints for the next word based on continuing or
     * starting at the added word.
     */
    void Advance(
        const strategy::phrase::Word &word,
        const strategy::horizon::Hypothesis &new_coverage,
        unsigned int advancing_engine,
        unsigned int word_offset) {
      unsigned int new_frontier = new_coverage.Frontier(advancing_engine);
      if (engine_ != UINT_MAX) {
        assert(engine_ == advancing_engine);
        // In a phrase and it hasn't ended.
        if (new_frontier <= end_) return;
      }

      // Don't enter a phrase if it's already broken.
      if (new_frontier > word_offset + 1) {
        end_ = 0;
        engine_ = UINT_MAX;
        return;
      }

      unsigned int max_end = new_coverage.MaxClear(advancing_engine);
      const strategy::phrase::Word::EndMap &ends = word.Ends();
      strategy::phrase::Word::EndMap::const_iterator beyond(ends.upper_bound(max_end));
      // All phrases longer than allowed by coverage.
      if (beyond == ends.begin()) {
        end_ = 0;
        engine_ = UINT_MAX;
        return;
      }
      --beyond;
      assert(beyond->first <= max_end);
      end_ = beyond->first;
      assert(end_ >= new_frontier);
      engine_ = advancing_engine;
    }

  private:
    unsigned int engine_, end_;
};

inline bool operator==(const Hypothesis &left, const Hypothesis &right) {
  return (left.Engine() == right.Engine()) && (left.End() == right.End());
}

inline bool operator!=(const Hypothesis &left, const Hypothesis &right) {
  return (left.Engine() != right.Engine()) || (left.End() != right.End());
}

inline size_t hash_value(const Hypothesis &phrase) {
  size_t seed = 0;
  boost::hash_combine(seed, phrase.Engine());
  boost::hash_combine(seed, phrase.End());
  return seed;
}

} // namespace phrase
} // namespace strategy

#endif
