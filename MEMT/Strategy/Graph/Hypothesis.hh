#ifndef _MEMT_Strategy_Graph_Hypothesis_h
#define _MEMT_Strategy_Graph_Hypothesis_h

#include "MEMT/Strategy/Graph/Coverage/Hypothesis.hh"
#include "MEMT/Strategy/Scorer/Hypothesis.hh"

#include <boost/functional/hash.hpp>

namespace strategy {
namespace graph {

template <class LanguageModelT> class Sentence;

template <class LanguageModelT> class Hypothesis;

template <class LanguageModel> bool operator==(const Hypothesis<LanguageModel> &left, const Hypothesis<LanguageModel> &right);

template <class LanguageModel> size_t hash_value(const Hypothesis<LanguageModel> &hyp);

template <class LanguageModelT> class Hypothesis {
  public:
    typedef LanguageModelT LanguageModel;

    Hypothesis() {}

    static const bool kLocationRelevant = true;

    size_t NGramLength() const { return scorer_.NGramLength(); }

  private:
    friend class Sentence<LanguageModel>;
    friend bool operator==<>(const Hypothesis<LanguageModel> &left, const Hypothesis<LanguageModel> &right);
    friend size_t hash_value<>(const Hypothesis<LanguageModel> &hyp);

    coverage::Hypothesis coverage_;
    scorer::Hypothesis<LanguageModel> scorer_;
};

template <class LanguageModel> bool operator==(const Hypothesis<LanguageModel> &left, const Hypothesis<LanguageModel> &right) {
  return (left.coverage_ == right.coverage_) && (left.scorer_ == right.scorer_);
}

template <class LanguageModel> size_t hash_value(const Hypothesis<LanguageModel> &hyp) {
  size_t ret = 0;
  boost::hash_combine(ret, hyp.coverage_);
  boost::hash_combine(ret, hyp.scorer_);
  return ret;
}

} // namespace graph
} // namespace strategy

#endif // _MEMT_Strategy_Graph_Hypothesis_h
