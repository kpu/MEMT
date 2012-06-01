#ifndef _MEMT_Strategy_Horizon_Horizon_h
#define _MEMT_Strategy_Horizon_Horizon_h

#include "MEMT/Strategy/Horizon/Config.hh"

#include <assert.h>
#include <vector>

namespace input { class Input; }

namespace strategy {
namespace horizon {

struct Range {
  unsigned int start;
  unsigned int end;
  bool Contains(unsigned int value) const {
    return (start <= value) && (value <= end);
  }
};

class Word {
  public:
    Word() {}

    unsigned int NumEngines() const {
      return range_.size();
    }

    const Range &GetBound(unsigned int other_engine) const {
      assert(other_engine < range_.size());
      return range_[other_engine];
    }

    // TODO: make private friended.
    void SetNumEngines(unsigned int num) {
      range_.resize(num);
    }

    void SetBound(unsigned int other_engine, unsigned int start, unsigned int end) {
      assert(start <= end);
      assert(other_engine < range_.size());
      range_[other_engine].start = start;
      range_[other_engine].end = end;
    }

  private:
    std::vector<Range> range_;
};

class System {
  public:
    System() {}

    const Word &operator[](size_t w) const { return words_[w]; }
    
    size_t size() const { return words_.size(); }

  protected:
    friend class Sentence;
    void Reset(const Config &config, const input::Input &input, size_t sys_num);

  private:
    std::vector<Word> words_;
};

class Sentence {
  public:
    Sentence() {}

    // Precondition: alignments have been made.
    void Reset(const Config &config, const input::Input &input);

    const System &operator[](size_t s) const { return systems_[s]; }

    const Config &GetConfig() const { return config_; }

    size_t size() const { return systems_.size(); }

  private:
    Config config_;

    std::vector<System> systems_;
};

} // namespace horizon
} // namespace strategy

#endif
