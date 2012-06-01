#ifndef _MEMT_Strategy_Sentence_h
#define _MEMT_Strategy_Sentence_h

#include "MEMT/Feature/Scorer/Sentence.hh"
#include "MEMT/Input/Location.hh"
#include "MEMT/Strategy/Legacy/Hypothesis.hh"

#include <boost/mpl/vector.hpp>

namespace decoder { class Score; }
namespace input { class Input; }

namespace strategy {
namespace legacy {

template <class FeaturesT> class Sentence {
  public:
    typedef FeaturesT Features;

    typedef typename ::strategy::legacy::Hypothesis<Features> Hypothesis;

    template <class ScorerInit> explicit Sentence(const ScorerInit &init) : input_(NULL), phrase_(), horizon_(), scorer_(init) {}

    // TODO: make Configure/Reset behavior standard across classes.
    void Reset(const Config &config, const input::Input &input) {
      input_ = &input;
      horizon_.Reset(config.horizon, input);
      phrase_.Reset(config.phrase, input);
      scorer_.Reset(config.scorer, input);
      config_ = config.legacy;
    }

    void Begin(Hypothesis &hypothesis, decoder::Score &scores) const {
      assert(input_);

      hypothesis.MutablePhrase().InitializeRoot();

      hypothesis.MutableHorizon().InitializeRoot(input_->NumEngines());

      scorer_.Begin(hypothesis.MutableScorer(), scores);
    }

    template <class Callback> void GetExtensions(
        const decoder::HypHistory *history,
        const Hypothesis &of,
        Callback out) const {
      if (of.Phrase().InPhrase()) {
        input::Location location;
        location.engine = of.Phrase().Engine();
        location.offset = of.Horizon().Frontier(location.engine);
        out(location);
        return;
      }

      std::set<input::Location> locations;

      // Get alignments of previously appended word.
      const input::WordAlignments &pre_align = input_->GetWord(history->Entry().engine, history->Entry().offset).alignments;
      for (unsigned int engine = 0; engine < input_->engines.size(); ++engine) {
        unsigned int frontier = of.Horizon().Frontier(engine);
        if (frontier < input_->engines[engine].Length()) {
          locations.insert(input::Location(engine, frontier));

          if (config_.extend_aligned) {
            for (unsigned int other = 0; other < input_->engines.size(); ++other) {
              const input::AlignmentInfo &info = input_->GetWord(engine, frontier).alignments.Ask(other);
              if (info.type & ~(input::AL_BOUNDARY | input::AL_SELF | input::AL_EXACT)) {
                locations.insert(input::Location(other, info.offset));
              }
            }
          }
        }
        if (config_.continue_recent) {
          const input::AlignmentInfo &info = pre_align.Ask(engine);
          if (info.IsConnection()) {
            unsigned int next_offset = info.offset + 1;
            if ((next_offset != frontier) && 
                (next_offset < input_->engines[engine].Length()) &&
                !of.Horizon().IsUsed(engine, next_offset)) {
              locations.insert(input::Location(engine, next_offset));
            }  
          }
        }
      }

      for (std::set<input::Location>::const_iterator i = locations.begin(); i != locations.end(); ++i) {
        assert(i->engine < input_->engines.size());
        assert(i->offset < input_->engines[i->engine].words.size());
        out(*i);
      }
    }

    // Minimially extend so hypothesis can be scored.
    void ExtendScore(
        const decoder::HypHistory *history,
        const input::Location &location,
        const Hypothesis &from_hypothesis,
        const decoder::Score &from_score,
        Hypothesis &to_hypothesis,
        decoder::Score &to_score) const {
      scorer_.Extend(history, location, from_hypothesis.Scorer(), from_score, to_hypothesis.MutableScorer(), to_score);
    }

    bool ExtendNonScore(
        const input::Location &location,
        const Hypothesis &from,
        Hypothesis &to) const {
      to.MutableHorizon() = from.Horizon();
      if (!to.MutableHorizon().ConsumeWord(*input_, horizon_, phrase_, location)) return false;
      to.MutablePhrase() = from.Phrase();
      to.MutablePhrase().Advance(
          phrase_[location.engine][location.offset],
          to.Horizon(),
          location.engine,
          location.offset);
      return true;
    }

    void End(
        size_t length,
        bool length_normalize,
        decoder::Score &adjust,
        std::vector<LogScore> &end_features) const {
      scorer_.End(length, length_normalize, adjust, end_features);
    }

  private:
    const input::Input *input_;

    phrase::Sentence phrase_;
    horizon::Sentence horizon_;
    feature::scorer::Sentence<Features> scorer_;
    LegacyOnlyConfig config_;
};

} // namespace legacy
} // namespace strategy

#endif // _MEMT_Strategy_Sentence_h
