#ifndef _MEMT_Strategy_Legacy_Hypothesis_h
#define _MEMT_Strategy_Legacy_Hypothesis_h

#include "MEMT/Strategy/Horizon/Hypothesis.hh"
#include "MEMT/Strategy/Phrase/Hypothesis.hh"
#include "MEMT/Feature/Scorer/Hypothesis.hh"

namespace strategy {
namespace legacy {

template <class Features> class Hypothesis;
template <class Features> class Sentence;
template <class Features> inline size_t hash_value(const Hypothesis<Features> &hyp);
template <class Features> inline bool operator==(
    const Hypothesis<Features> &left,
    const Hypothesis<Features> &right);

template <class FeaturesT> class Hypothesis {
  public:
    typedef FeaturesT Features;
    typedef typename feature::scorer::Hypothesis<Features>::type ScorerState;

    static const bool kLocationRelevant = false;

    Hypothesis() {}

  private:
    const horizon::Hypothesis &Horizon() const { return horizon_; }
    horizon::Hypothesis &MutableHorizon() { return horizon_; }

    const phrase::Hypothesis &Phrase() const { return phrase_; }
    phrase::Hypothesis &MutablePhrase() { return phrase_; }

    const ScorerState &Scorer() const { return scorer_; }
    ScorerState &MutableScorer() { return scorer_; }

    friend size_t hash_value<>(const Hypothesis<Features> &hyp);
    friend bool operator==<>(const Hypothesis<Features> &left, const Hypothesis<Features> &right);
    friend class Sentence<Features>;

    horizon::Hypothesis horizon_;
    phrase::Hypothesis phrase_;
    ScorerState scorer_;
};

template <class Features> inline bool operator==(
    const Hypothesis<Features> &left,
    const Hypothesis<Features> &right) {
  return
    (left.horizon_ == right.horizon_) &&
    (left.phrase_ == right.phrase_) &&
    (left.scorer_ == right.scorer_);
}

template <class Features> inline bool operator!=(
    const Hypothesis<Features> &left,
    const Hypothesis<Features> &right) {
  return !(left == right);
}

template <class Features> inline size_t hash_value(const Hypothesis<Features> &hyp) {
  size_t ret = 0;
  boost::hash_combine(ret, hyp.horizon_);
  boost::hash_combine(ret, hyp.phrase_);
  boost::hash_combine(ret, hyp.scorer_);
  return ret;
}

} // namespace legacy
} // namespace strategy

#endif // _MEMT_Strategy_Legacy_Hypothesis_h
