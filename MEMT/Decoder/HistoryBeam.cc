#include "MEMT/Decoder/HistoryBeam.hh"

#include <algorithm>

namespace decoder {

void MergeSizeOneHistoryBeam(HistoryBeam &into, const HistoryBeam &with) {
  assert(with.size() == 1);
  const boost::shared_ptr<HypHistory> &hist = *with.unordered_begin();
  if (into.MayMakeIt(hist)) {
    into.Available() = hist;
    into.InsertAvailable();
  }
}

void DumpBeamToHypHistory(HistoryBeam &in, HypHistory &out) {
  HypHistory::Previous &previous = out.MutablePrevious();
  previous.clear();
  previous.reserve(in.size());
  
  in.destructive_ordered_make();
  for (HistoryBeam::decreasing_iterator i = in.destructive_decreasing_begin(); i != in.destructive_decreasing_end(); ++i) {
    previous.push_back(*i);
  }
  out.MakeHash();
}

} // namespace decoder
