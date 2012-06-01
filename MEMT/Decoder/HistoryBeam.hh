#include "MEMT/Decoder/History.hh"
#include "util/n_best.hh"

#include <functional>

#include <boost/shared_ptr.hpp>

/* When hypotheses are LM dupe detected, they get passed to this beam, which 
 * does a secondary full equality dupe removal and packs the hypotheses.
 */

namespace decoder {

namespace detail {

struct HistoryLess : public std::binary_function<const boost::shared_ptr<HypHistory> &, const boost::shared_ptr<HypHistory> &, bool> {
  HistoryLess() : less_() {}

  bool operator()(const boost::shared_ptr<HypHistory> &left, const boost::shared_ptr<HypHistory> &right) const {
    return less_(left->Entry(), right->Entry());
  }
  private:
    const HistoryEntry::LessByScore less_;
};

} // namespace detail

typedef nbest::NBest<
  boost::shared_ptr<HypHistory>,
  detail::HistoryLess,
  nbest::HashDupe<boost::shared_ptr<HypHistory>, HypHistory::ReturnHash, HypHistory::EqualsHashOnly>,
  nbest::OneBestMerge<boost::shared_ptr<HypHistory>, detail::HistoryLess> > HistoryBeam;

void MergeSizeOneHistoryBeam(HistoryBeam &into, const HistoryBeam &with);

void DumpBeamToHypHistory(HistoryBeam &in, HypHistory &out);

} // namespace decoder
