#ifndef _MEMT_Strategy_Horizon_Hypothesis_h
#define _MEMT_Strategy_Horizon_Hypothesis_h

#include "util/numbers.hh"

#include <boost/functional/hash.hpp>

#include <algorithm>
#include <ostream>
#include <vector>

namespace input { class Input; class Location; }

namespace strategy {
namespace phrase { class Sentence; }
namespace horizon {

class Sentence;

// A hypothesis's view of one engine's input.
class Coverage {
  public:
    Coverage() {}

    void InitializeRoot() {
      // Skip beginning of sentence.
      frontier_ = 1;
      used_.clear();
    }

    // Faster version of Use(Frontier())
    void UseFrontier();

    // Mark the word at offset as used, advancing Frontier() if Frontier() == offset
    void Use(unsigned int offset);

    // Make Frontier() at least word.  Return true if changed.
    bool UseTo(unsigned int offset);

    // Is the word at offset used?
    bool IsUsed(unsigned int offset) const {
      if (offset < frontier_) return true;
      if (offset == frontier_) return false;
      unsigned int used_idx = UsedIdx(offset);
      if (used_.size() <= used_idx) return false;
      return used_[used_idx];
    }

    // Is the interval [Frontier(), offset] available?
    bool IsClearThrough(unsigned int offset) const {
      if (offset < frontier_) return false;
      std::vector<bool>::const_iterator ending =
        used_.begin() + std::min<size_t>(offset - frontier_, used_.size());
      return std::find(used_.begin(), ending, true) == ending;
    }

    // Return the maximum offset for which IsClearThrough(offset).  
    // This is also the value returned by 
    // unsigned int Peek(const Coverage &peek) {
    //   Coverage tmp = peek;
    //   tmp.UseFrontier();
    //   return tmp.Frontier();
    // }
    // except it's more efficient.
    unsigned int MaxClear() const {
      return std::find(used_.begin(), used_.end(), false) - used_.begin() + 1 + frontier_;
    }

    // Index of next available word.
    unsigned int Frontier() const { return frontier_; }

  private:
    friend std::ostream &operator<<(std::ostream &s, const Coverage &coverage);
    friend size_t hash_value(const Coverage &coverage);
    friend bool operator==(const Coverage &left, const Coverage &right);
    friend bool operator!=(const Coverage &left, const Coverage &right);

    unsigned int UsedIdx(unsigned int offset) const {
      return offset - frontier_ - 1;
    }

    unsigned int frontier_;
    // used_[UsedIdx(word)] is true iff the word has been used by an alignment.
    // Implicitly, use_ is false beyond its boundary.
    std::vector<bool> used_;
};

std::ostream &operator<<(std::ostream &s, const Coverage &coverage);

inline bool operator==(const Coverage &left, const Coverage &right) {
  return (left.frontier_ == right.frontier_) && (left.used_ == right.used_);
}

inline bool operator!=(const Coverage &left, const Coverage &right) {
  return (left.frontier_ != right.frontier_) || (left.used_ != right.used_);
}

inline size_t hash_value(const Coverage &coverage) {
  size_t seed = 0;
  boost::hash_combine(seed, coverage.Frontier());
  boost::hash_combine(seed, coverage.used_);
  return seed;
}

// Coverage for all inputs.  This never provides external access to a Coverage class.
class Hypothesis {
  public:
    void InitializeRoot(unsigned int engines) {
      inputs_.resize(engines);
      for (std::vector<Coverage>::iterator i = inputs_.begin(); i != inputs_.end(); ++i) {
        i->InitializeRoot();
      }
    }

    // The only coverage queries intended for external consumption.
    unsigned int MaxClear(unsigned int engine) const {
      assert(engine < inputs_.size());
      return inputs_[engine].MaxClear();
    }

    unsigned int Frontier(unsigned int engine) const {
      assert(engine < inputs_.size());
      return inputs_[engine].Frontier();
    }

    bool IsUsed(unsigned int engine, unsigned int offset) const {
      return inputs_[engine].IsUsed(offset);
    }

    // Use the word on Frontier(engine).  This may cause any engine to run off
    // the end of sentence.  Returns false iff all engines run off.
    bool ConsumeWord(const input::Input &text, const Sentence &sentence, const strategy::phrase::Sentence &phrase, const input::Location &location);

  private:
    friend std::ostream &operator<<(std::ostream &s, const Hypothesis &coverage);
    friend size_t hash_value(const Hypothesis &all_coverage);
    friend bool operator==(const Hypothesis &left, const Hypothesis &right);
    friend bool operator!=(const Hypothesis &left, const Hypothesis &right);

    void Alignments(const input::Input &text, const strategy::phrase::Sentence &phrase, const input::Location &location);

    void SkipLeadingPunctuation(const input::Input &text, unsigned int engine);

    void EnforceNewConstraints(const input::Input &text, const Sentence &sentence);
    void EnforceOldConstraints(const input::Input &text, const Sentence &sentence);

    std::vector<Coverage> inputs_;
};

std::ostream &operator<<(std::ostream &s, const Hypothesis &coverage);

inline bool operator==(const Hypothesis &left, const Hypothesis &right) {
  return left.inputs_ == right.inputs_;
}

inline bool operator!=(const Hypothesis &left, const Hypothesis &right) {
  return left.inputs_ != right.inputs_;
}

inline size_t hash_value(const Hypothesis &all_coverage) {
  boost::hash<std::vector<Coverage> > hasher;
  return hasher(all_coverage.inputs_);
}

} // namespace horizon
} // namespace strategy

#endif
