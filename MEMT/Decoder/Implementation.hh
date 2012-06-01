#ifndef _MEMT_Decoder_Implementation_h
#define _MEMT_Decoder_Implementation_h

#include "MEMT/Decoder/InternalBeam.hh"
#include "MEMT/Decoder/EndBeam.hh"
#include "MEMT/Decoder/Config.hh"
#include "MEMT/Decoder/History.hh"
#include "MEMT/Input/Input.hh"
#include "MEMT/Strategy/Horizon/Horizon.hh"
#include "lm/word_index.hh"

#include<boost/bind.hpp>
#include<boost/functional/hash.hpp>
#include<boost/shared_ptr.hpp>

#include<algorithm>
#include<vector>

namespace decoder {

using namespace std;

/* Core of the decoder.
 * Maintains three beams: current (established hypotheses), next (current plus a word), and ending (end of sentence).
 */
template <class StrategyT> class DecoderImpl {
  private:
    typedef StrategyT Strategy;
    typedef InternalHypothesis<typename Strategy::Hypothesis> InternalHypothesisT;
    typedef typename InternalBeam<InternalHypothesisT>::T InternalBeamT;
    typedef EndingHypothesis EndingHypothesisT;

  public:
    DecoderImpl() 
      : current_(&alternate1_),
        next_(&alternate2_),
        scratch_history_(new FastHypHistory) {}

    template <class BeamDumperT> void Run(
        const Config &config,
        const input::Input &input,
        const Strategy &strategy,
        BeamDumperT &dumper,
        std::vector<CompletedHypothesis> &nbest) {

      length_ = 1;
      config_ = &config;
      input_ = &input;

      strategy_ = &strategy;
            
      assert(!input.engines.empty());
      assert(!input.engines.front().words.empty());

      current_->Reset(config.internal_beam_size);
      next_->Reset(config.internal_beam_size);
      ResetEndBeam(ending_, config.end_beam_size);

      current_->Available().InitializeRoot(
          input.engines.front().words.front().text.CanonicalHash(),
          config_->end_beam_size,
          *strategy_);
      current_->InsertAvailable();
      current_->destructive_ordered_make();
      while (!current_->empty()) {
        dumper.DumpBeam(length_++, *current_);
        ExtendBeam();
      }
      MakeCompletedHypotheses(ending_, config_->length_normalize, nbest);
      // This isn't required, but it does free up a bunch of memory at once, increasing the likelihood of finding big blocks.   
      for (typename InternalBeamT::unordered_iterator i = current_->unordered_begin(); i != current_->unordered_end(); ++i)
        i->Clear();
      for (typename InternalBeamT::unordered_iterator i = next_->unordered_begin(); i != next_->unordered_end(); ++i)
        i->Clear();
      for (typename EndBeam::unordered_iterator i = ending_.unordered_begin(); i != ending_.unordered_end(); ++i)
        i->Clear();
      scratch_history_->Reset();
    }

  private:
    // Utility for ExtendHypothesisByLocation.
    HypHistory *UseScratchHistory() {
      HypHistory *ret = scratch_history_.release();
      scratch_history_.reset(new FastHypHistory);
      return ret;
    }

    void ExtendHypothesisByLocation(
        const InternalHypothesisT &start,
        const input::Location &location);

    /* Take a hypothesis and consider all advancements by a word.  If that word is 
     * end of sentence, put the new hypothesis into ending.  Otherwise put it into
     * continuing.
     */
    void ExtendHypothesis(const InternalHypothesisT &start) {
      strategy_->GetExtensions(start.History().get(), start.GetPayload(), boost::bind<void>(&DecoderImpl<StrategyT>::ExtendHypothesisByLocation, this, boost::ref(start), _1));
    }

    void ExtendBeam() {
      for (typename InternalBeamT::decreasing_iterator i = current_->destructive_decreasing_begin();
          i != current_->destructive_decreasing_end(); ++i) {
        ExtendHypothesis(*i);
      }
      for (typename InternalBeamT::unordered_iterator i = next_->unordered_begin(); i != next_->unordered_end(); ++i) {
        i->FinishMerging();
      }
      next_->destructive_ordered_make();
      current_->Reset(config_->internal_beam_size);
      swap(current_, next_);
    }

    const Config *config_;
    const input::Input *input_;

    const Strategy *strategy_;

    InternalBeamT alternate1_, alternate2_;
    // current_ and next_ point into alternate_ and swap after each is used.
    InternalBeamT *current_, *next_;

    EndBeam ending_;

    // Used by ExtendHypothesis by engine as a scratch history.  
    // If the hypothesis makes it into the beam, a new HypHistory
    // is created.
    std::auto_ptr<HypHistory> scratch_history_;

    unsigned int length_;
};

template <class StrategyT> void DecoderImpl<StrategyT>::ExtendHypothesisByLocation(
    const InternalHypothesisT &start,
    const input::Location &location) {
  scratch_history_->Reset();
  HistoryEntry &scratch_entry = scratch_history_->MutableEntry();
  scratch_entry.engine = location.engine;
  scratch_entry.offset = location.offset;
  // TODO: consider assigning separate word indices to unseen words.
  const input::Word &word = input_->engines[location.engine].words[location.offset];
  scratch_entry.word = word.text.CanonicalHash();

  assert(start.History().get());
  strategy_->ExtendScore(
      start.History().get(),
      location,
      start.GetPayload(),
      start.GetScore(),
      // No harm done if end of sentence.
      next_->Available().MutablePayload(),
      scratch_entry.score);  

  // End of sentence condition.
  if (word.text.IsEnd()) {
    strategy_->End(length_, config_->length_normalize, scratch_entry.score, ending_.Available().MutableEndFeatures());
    if (!ending_.MayMakeIt(scratch_entry.score)) return;
    ending_.Available().Set(UseScratchHistory(), start.History(), length_);
    ending_.InsertAvailable();
    return;
  }

  if (!next_->MayMakeIt(scratch_entry.score)) return;
  if (!next_->Available().ExtendAllButScorer(
      start,
      *strategy_,
      config_->end_beam_size,
      UseScratchHistory())) return;
  next_->InsertAvailable();
}

} // namespace decoder

#endif // _MEMT_Decoder_Implementation_h
