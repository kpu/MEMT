#ifndef _MEMT_Strategy_Graph_Sentence_h
#define _MEMT_Strategy_Graph_Sentence_h

#include "MEMT/Strategy/Graph/Config.hh"
#include "MEMT/Strategy/Graph/Hypothesis.hh"
#include "MEMT/Strategy/Graph/Coverage/Sentence.hh"
#include "MEMT/Strategy/Scorer/Sentence.hh"

namespace strategy {
namespace graph {

template <class LanguageModelT> class Sentence {
  public:
    typedef LanguageModelT LanguageModel;
    typedef graph::Hypothesis<LanguageModel> Hypothesis;

    explicit Sentence(const LanguageModel &lm) : input_(NULL), coverage_(), scorer_(lm) {}

    void Reset(const Config &config, const input::Input &input) {
      input_ = &input;
      coverage_.Reset(config.coverage, input);
      scorer_.Reset(config.scorer, input);
    }

    void Begin(Hypothesis &hypothesis, decoder::Score &scores) const {
      assert(input_);
      coverage_.Begin(hypothesis.coverage_);
      scorer_.Begin(hypothesis.scorer_, scores);
    }

    template <class Callback> void GetExtensions(
        const decoder::HypHistory *history,
        const Hypothesis &of,
        Callback out) const {
      coverage_.GetExtensions(history, of.coverage_, out);
    }

      // Minimially extend so hypothesis can be scored.
    void ExtendScore(
        const decoder::HypHistory *history,
        const input::Location &location,
        const Hypothesis &from_hypothesis,
        const decoder::Score &from_score,
        Hypothesis &to_hypothesis,
        decoder::Score &to_score) const {
      scorer_.Extend(history, location, from_hypothesis.scorer_, from_score, to_hypothesis.scorer_, to_score);
    }

    bool ExtendNonScore(
        const input::Location &location,
        const Hypothesis &from,
        Hypothesis &to) const {
      return coverage_.Extend(location, from.coverage_, to.coverage_);
    }

    void End(
        size_t length,
        bool length_normalize,
        decoder::Score &adjust) const {
      scorer_.End(length, length_normalize, adjust);
    }

  private:
    const input::Input *input_;

    coverage::Sentence coverage_;
    scorer::Sentence<LanguageModel> scorer_;
};

} // namespace graph
} // namespace strategy

#endif // _MEMT_Strategy_Graph_Sentence_h
