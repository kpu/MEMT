#ifndef _MEMT_Feature_Verbatim_Sentence_h
#define _MEMT_Feature_Verbatim_Sentence_h

#include "MEMT/Feature/Verbatim/Config.hh"
#include "MEMT/Feature/Verbatim/Hypothesis.hh"

#include "util/numbers.hh"

#include <algorithm>
#include <vector>

namespace input { class Input; class Location; }
namespace decoder { class HypHistory; }

namespace feature {
namespace verbatim {

class Sentence {
  public:
    typedef ::feature::verbatim::Config Config;
    typedef ::feature::verbatim::Hypothesis Hypothesis;
    
    Sentence() {}

    void Reset(const Config &config, const input::Input &input) {
      config_ = config;
    }

    size_t BothFeatures() const {
      return config_.FeatureCount();
    }
    size_t EndFeatures() const { return 0; }

    void Begin(Hypothesis &start_state, LogScore *start_scores) const;

    void Extend(
        const input::Input &input,
        const decoder::HypHistory *history,
        const input::Location &append,
        const Hypothesis &from_state,
        const LogScore *from_scores,
        Hypothesis &to_state,
        LogScore *to_scores) const;

    void End(size_t length, LogScore *out) const {}

  private:
    Config config_;
};

} // namespace verbatim
} // namespace feature

#endif // _MEMT_Feature_Verbatim_Sentence_h
