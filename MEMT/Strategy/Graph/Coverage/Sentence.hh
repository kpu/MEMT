#ifndef _MEMT_Strategy_Graph_Coverage_Sentence_h
#define _MEMT_Strategy_Graph_Coverage_Sentence_h

#include "MEMT/Decoder/History.hh"
#include "MEMT/Input/Input.hh"
#include "MEMT/Strategy/Graph/Coverage/Hypothesis.hh"

#include <vector>

namespace strategy {
namespace graph {
namespace coverage {

/* This is a slow and kludgy version of the graph search space.  Specifically,
 * we do not have transitive alignments yet.  Therefore coverage is maintained
 * at the word level.
 */

class Sentence {
  public:
    typedef coverage::Hypothesis Hypothesis;

    void Reset(const Config &config, const input::Input &input) {
      input_ = &input;
      engine_to_bitstart_.clear();
      size_t start = 0;
      for (size_t e = 0; e < input_->NumEngines(); ++e) {
        engine_to_bitstart_.push_back(start);
        start += input_->engines[e].words.size();
      }
      bit_size_ = start;
    }

    void Begin(Hypothesis &hypothesis) const {
      assert(input_);
      hypothesis.bits_.clear();
      hypothesis.bits_.resize(bit_size_);
    }

    template <class Callback> void GetExtensions(
        const decoder::HypHistory *history,
        const Hypothesis &of,
        Callback out) const { 
      const input::WordAlignments &pre_align = input_->GetWord(history->Entry().engine, history->Entry().offset).alignments;
      // Each word aligned with the previously appended word
      for (input::WordAlignments::const_iterator pre = pre_align.begin(); pre != pre_align.end(); ++pre) {
        if (pre->IsNone()) continue;
        // Get word after the aligned one (the bigram continuation).  The decoder will not ask to extend an ended hypothesis, so an extension exists.
        const input::WordAlignments &cont_align = input_->GetWord(pre - pre_align.begin(), pre->offset + 1).alignments;
        for (input::WordAlignments::const_iterator cont = cont_align.begin(); cont != cont_align.end(); ++cont) {
          if (cont->IsNone()) continue;
          input::Location extend(cont - cont_align.begin(), cont->offset);
          if (!of.bits_.test(Bit(extend.engine, extend.offset))) {
            out(extend);
          }
        }
      }
    }

    bool Extend(
        const input::Location &location,
        const Hypothesis &from,
        Hypothesis &to) const {
      to = from;
      const input::WordAlignments &alignments = input_->GetWord(location).alignments;
      for (input::WordAlignments::const_iterator al = alignments.begin(); al != alignments.end(); ++al) {
        if (al->IsConnection()) {
          to.bits_.set(Bit(al - alignments.begin(), al->offset));
        }
      }
      return true;
    }

  private:
    size_t Bit(size_t engine, size_t offset) const {
      return engine_to_bitstart_[engine] + offset;
    }

    const input::Input *input_;
    std::vector<size_t> engine_to_bitstart_;

    size_t bit_size_;
};

} // namespace coverage
} // namespace graph
} // namespace strategy

#endif // _MEMT_Strategy_Graph_Coverage_Sentence_h
