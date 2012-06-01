#ifndef _MEMT_Decoder_EndBeam_h
#define _MEMT_Decoder_EndBeam_h

#include "MEMT/Decoder/Completed.hh"
#include "MEMT/Decoder/Hypothesis.hh"

#include "util/n_best.hh"

#include <functional>
#include <vector>

namespace decoder {

namespace detail {

struct EndingHypReturnHash : public std::unary_function<const EndingHypothesis &, size_t> {
  size_t operator()(const EndingHypothesis &hyp) const {
    return hyp.HistoryHash();
  }
};

// Computing full equals of a hypothesis is really expensive, so we just hope there is no hash collision.  
// There thousands of sentence end hypotheses, so the probability of collision is very small.
struct EqualsHashOnly : public std::binary_function<const EndingHypothesis &, const EndingHypothesis &, bool> {
  size_t operator()(const EndingHypothesis &left, const EndingHypothesis &right) const {
    return left.HistoryHash() == right.HistoryHash();
  }
};

}  // namespace detail

/* The EndBeam is the final (rightmost) node in the lattice, representing end of sentence.  
 * Unlike all other nodes, the incoming edges come from hypotheses of different length.
 * This means it uses length normalization to sort.
 */
typedef nbest::NBest<
  EndingHypothesis,
  EndingHypothesis::LessByOverall,
  nbest::HashDupe<EndingHypothesis, detail::EndingHypReturnHash, detail::EqualsHashOnly>,
  nbest::OneBestMerge<EndingHypothesis, EndingHypothesis::LessByOverall>
  > EndBeam;

void ResetEndBeam(EndBeam &beam, size_t max_size);

void MakeCompletedHypotheses(const EndBeam &beam, bool length_normalize, std::vector<CompletedHypothesis> &nbest);

} // namespace decoder

#endif
