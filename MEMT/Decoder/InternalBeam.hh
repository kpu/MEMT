#ifndef _MEMT_Decoder_InternalBeam_h
#define _MEMT_Decoder_InternalBeam_h

#include "util/n_best.hh"

#include <functional>

namespace decoder {

namespace detail {

template <class InternalHypothesisT> struct CallMerge : public std::binary_function<InternalHypothesisT &, const InternalHypothesisT &, bool> {
  bool operator()(InternalHypothesisT &to, const InternalHypothesisT &with) const {
    return to.Merge(with);
  }
};

}  // namespace detail

template <class Hyp> struct InternalBeam {
  typedef nbest::NBest<
    Hyp,
    typename Hyp::LessByOverall,
    nbest::HashDupe<Hyp, typename Hyp::ReturnCachedHash, typename Hyp::EqualsUpToLM>,
    detail::CallMerge<Hyp>
  > T;
};

} // namespace decoder

#endif

