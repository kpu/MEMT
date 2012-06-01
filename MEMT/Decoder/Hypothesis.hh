#ifndef _MEMT_Decoder_Hypothesis_h
#define _MEMT_Decoder_Hypothesis_h
// See also Completed.hh for the final output hypothesis produced by the decoder.

#include "MEMT/Decoder/History.hh"
#include "MEMT/Decoder/HistoryBeam.hh"
#include "MEMT/Strategy/Phrase/Hypothesis.hh"
#include "MEMT/Decoder/Score.hh"
#include "MEMT/Input/Input.hh"
#include "util/n_best.hh"

#include<boost/functional/hash.hpp>
#include<boost/shared_ptr.hpp>
#include<boost/noncopyable.hpp>

#include<functional>

namespace decoder {

/* Base hypothesis with score and history.  This is used by both intermediate and ending hypotheses.  
 * Intended to be manipulated by ExtendHypothesis.  Be sure to update it when adding new members.
 */
class BaseHypothesis {
  public:
    struct LessByOverall : public std::binary_function<const BaseHypothesis &, const BaseHypothesis &, bool> {
      LessByOverall() {}

      bool operator()(const BaseHypothesis &left, const BaseHypothesis &right) const {
        return left.Overall() < right.Overall();
      }
      bool operator()(const BaseHypothesis &left, const Score &right) const {
        return left.Overall() < right.Overall();
      }
    };

    BaseHypothesis() {}

    // Not necessary, but useful for saving memory.  
    void Clear() {
      history_.reset();
    }

    // Convenience access to history entries without giving full access to history.
    const Score &GetScore() const {
      return history_->Entry().score;
    }

    const LogScore &Overall() const {
      return GetScore().Overall();
    }

    uint64_t Word() const {
      return history_->Entry().word;
    }

  protected:
    void InitializeRoot(uint64_t sentence_begin) {
      history_.reset(new FastHypHistory);
      history_->InitializeRoot(sentence_begin);
    }

    void GiveHistory(HypHistory *history) {
      history_.reset(history);
    }

    // Words decoded so far.
    boost::shared_ptr<HypHistory> history_;

};

template <class PayloadT> class InternalHypothesis : public BaseHypothesis, private boost::noncopyable {
  public:
    typedef PayloadT Payload;

    InternalHypothesis() : entry_merge_() {
      DEBUG_ONLY(finished_merging_ = false);
    }

    // Precondition: RehashUpToLM has been called on the hyp after anything impacting it has been changed.
    // In practice this is only used by the beam, which calls RehashUpToLM on entry of a hypothesis.
    struct ReturnCachedHash : public std::unary_function<const InternalHypothesis<Payload> &, size_t> {
      size_t operator()(const InternalHypothesis<Payload> &hyp) const {
        return hyp.HashUpToLM();
      }
    };

    /* Compare hypotheses to see if they have the same word history up to order,
     * the same phrase settings, and the same coverage.  This is used to detect
     * duplicate hypotheses in the beam.  They may have different scores: the idea
     * is to remove the lower scoring one.
     */
    struct EqualsUpToLM : public std::binary_function<const InternalHypothesis<Payload>, const InternalHypothesis<Payload>, bool> {
      bool operator()(
          const InternalHypothesis<Payload> &left, 
          const InternalHypothesis<Payload> &right) const {
        if (!(left.payload_ == right.payload_)) return false;
        if (Payload::kLocationRelevant) {
          const HistoryEntry &l = left.history_->Entry(), &r = right.history_->Entry();
          if (l.engine != r.engine) return false;
          if (l.offset != r.offset) return false;
        }
        return true;
      }
    };

    // Initialization
    template <class Strategy> void InitializeRoot(uint64_t begin_sentence, size_t top_n, const Strategy &strategy) {
      BaseHypothesis::InitializeRoot(begin_sentence);
      previous_.Reset(top_n);

      strategy.Begin(payload_, history_->MutableScore());

      RehashUpToLM();
      DEBUG_ONLY(finished_merging_ = true);
    }

    // Induction
    const Payload &GetPayload() const { return payload_; }
    Payload &MutablePayload() { return payload_; }

    // Takes ownership of history_entry_only
    // Returns false iff hypothesis is not viable i.e. all hypotheses ran off end of sentence.
    template <class Strategy> bool ExtendAllButScorer(
        const InternalHypothesis<Payload> &start,
        const Strategy &strategy,
        size_t top_n,
        HypHistory *history_entry_only) {
      GiveHistory(history_entry_only);
      const HistoryEntry &entry = history_entry_only->Entry();

      if (!strategy.ExtendNonScore(input::Location(entry.engine, entry.offset), start.GetPayload(), payload_)) {
        return false;
      }

      previous_.Reset(top_n);
      previous_.Available() = start.History();
      previous_.Available()->MakeHash();
      previous_.InsertAvailable();

      RehashUpToLM();
      DEBUG_ONLY(finished_merging_ = false);
      return true;
    }

    // Access that can happen before, during, and after merging.
    const HypHistory *AnyPrevious() const {
      return previous_.empty() ? NULL : previous_.unordered_begin()->get();
    }

    // Merging specific functions.

    size_t HashUpToLM() const {
      DEBUG_ONLY(assert(hash_up_to_lm_valid_));
      return hash_up_to_lm_;
    }

    // Precondition: EqualsUpToLM(*this, with);
    bool Merge(const InternalHypothesis &with) {
      DEBUG_ONLY(assert(!finished_merging_));
      MergeSizeOneHistoryBeam(previous_, with.previous_);
      return entry_merge_(history_->MutableEntry(), with.history_->Entry());
    }

    // Call this when all the merging is done.
    void FinishMerging() {
      DEBUG_ONLY(assert(!finished_merging_));
      DumpBeamToHypHistory(previous_, *history_);
      DEBUG_ONLY(finished_merging_ = true);
    }

    // Post-merging access.  Precondition: FinishMerging() was called.  
    const boost::shared_ptr<HypHistory> &History() const {
      DEBUG_ONLY(assert(finished_merging_));
      return history_;
    }

  private:
    void RehashUpToLM() {
      hash_up_to_lm_ = 0;
      boost::hash_combine(hash_up_to_lm_, payload_);
      if (Payload::kLocationRelevant) {
        boost::hash_combine(hash_up_to_lm_, history_->Entry().engine);
        boost::hash_combine(hash_up_to_lm_, history_->Entry().offset);
      }
      DEBUG_ONLY(hash_up_to_lm_valid_ = true);
    }

    DEBUG_ONLY(bool finished_merging_);

    HistoryBeam previous_;

    DEBUG_ONLY(bool hash_up_to_lm_valid_);
    
    size_t hash_up_to_lm_;

    const nbest::OneBestMerge<HistoryEntry, HistoryEntry::LessByScore> entry_merge_;

    Payload payload_;
};

class EndingHypothesis : public BaseHypothesis {
  public:
    EndingHypothesis() {}

    // This is separate from set so it can be passed to the scorer.  
    std::vector<LogScore> &MutableEndFeatures() {
      return end_features_;
    }

    // Precondition: history has Entry() set properly.
    // Takes ownership of history.
    void Set(HypHistory *history, const boost::shared_ptr<HypHistory> &previous, unsigned int length) {
      length_ = length;
      history->SetSinglePrevious(previous);
      history->MakeHash();
      BaseHypothesis::GiveHistory(history);
    }

    // Accessors
    // Precondition: Set has been called.
  
    size_t HistoryHash() const {
      return history_->Hash();
    }

    const boost::shared_ptr<HypHistory> &GetHistory() const {
      return history_;
    }

    unsigned int Length() const { return length_; }

    const std::vector<LogScore> &EndFeatures() const {
      return end_features_;
    }

  private:
    std::vector<LogScore> end_features_;

    unsigned int length_;
};

} // namespace decoder

#endif
