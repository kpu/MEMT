#ifndef _MEMT_Feature_Scorer_Hypothesis_h
#define _MEMT_Feature_Scorer_Hypothesis_h

#include "MEMT/Feature/LM/Hypothesis.hh"
#include "MEMT/Feature/Verbatim/Hypothesis.hh"

// Not used directly here, but clients expect hash_value.
#include "util/hash_fusion.hh"

#include <boost/fusion/algorithm.hpp>
// Not used directly here, but clients expect ==
#include <boost/fusion/sequence/comparison.hpp>
#include <boost/mpl/transform.hpp>

namespace feature {
namespace scorer {

namespace detail {
template <class T> struct HypothesisOp {
  typedef typename T::Hypothesis type;
};
} // namespace detail

template <class Sentences> struct Hypothesis {
  // Convert a vector of Sentence objects into their ::Hypothesis objects
  typedef typename boost::mpl::transform<Sentences, detail::HypothesisOp<boost::mpl::_1> >::type type;
};

} // namespace scorer
} // namespace feature

#endif // _MEMT_Feature_Scorer_Hypothesis_h
