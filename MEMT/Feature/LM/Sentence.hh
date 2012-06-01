#ifndef _MEMT_Feature_LM_Sentence_h
#define _MEMT_Feature_LM_Sentence_h

#include "MEMT/Decoder/History.hh"
#include "MEMT/Input/Location.hh"
#include "MEMT/Feature/LM/Config.hh"

#include "lm/virtual_interface.hh"
#include "util/numbers.hh"

#include <vector>

namespace lm { namespace base { class Vocabulary; } }
namespace input { class Input; }

namespace feature {
namespace lm {

void LookupVocab(const input::Input &in, const ::lm::base::Vocabulary &vocab, std::vector<std::vector< ::lm::WordIndex> > &indices);

/* scores are
 * log probability
 * Negative OOV count
 */

template <class LanguageModelT> class Sentence {
  public:
    typedef LanguageModelT LanguageModel;
    typedef ::feature::lm::Config Config;
    typedef std::vector<typename LanguageModel::State> Hypothesis;

    explicit Sentence(const std::vector<const LanguageModel *> &models) : models_(models) {}

    void Reset(const Config &config, const input::Input &input) {
      config_ = config;
      indices_.resize(models_.size());
      for (size_t i = 0; i < models_.size(); ++i) {
        LookupVocab(input, models_[i]->GetVocabulary(), indices_[i]);
      }
    }

    size_t BothFeatures() const {
      return config_.FeatureCount();
    }
    size_t EndFeatures() const { return 0; }

    void Begin(Hypothesis &start_state, LogScore *start_scores) const {
      start_state.resize(models_.size());
      for (size_t i = 0; i < models_.size(); ++i) {
        start_state[i] = models_[i]->BeginSentenceState();
      }
      for (LogScore *i = start_scores; i != start_scores + BothFeatures(); ++i) {
        i->SetLog(0.0);
      }
    }

    void Extend(
        const input::Input &input,
        const decoder::HypHistory *history,
        const input::Location &append,
        const Hypothesis &from_state,
        const LogScore *from_scores,
        Hypothesis &to_state,
        LogScore *to_scores) const {
      to_state.resize(models_.size());
      for (size_t i = 0; i < models_.size(); ++i) {
        ::lm::WordIndex word = indices_[i][append.engine][append.offset];

        ::lm::FullScoreReturn ret = models_[i]->FullScore(from_state[i], word, to_state[i]);
        *(to_scores++) = *(from_scores++) * LogScore(AlreadyLogTag(), ret.prob * M_LN10);
        if (word == models_[i]->GetVocabulary().NotFound()) {
          (to_scores++)->SetLog((from_scores++)->Log() + 1.0);
        } else {
          *(to_scores++) = *(from_scores++);
        }

        if (config_.by_length) {
          std::copy(from_scores, from_scores + models_[i]->Order() - 1, to_scores);
          if (ret.ngram_length != models_[i]->Order()) {
            // unk counts as a 1-gram for these purposes.  
            unsigned char impact = std::max<unsigned char>(1, ret.ngram_length) - 1;
            to_scores[impact].SetLog(from_scores[impact].Log() + 1.0);
          }
          from_scores += models_[i]->Order() - 1;
          to_scores += models_[i]->Order() - 1;
        }
      }
    }

    void End(size_t length, LogScore *out) const {}

  private:
    const std::vector<const LanguageModel *> models_;

    Config config_;

    std::vector<std::vector<std::vector< ::lm::WordIndex> > > indices_;
};

// Individual lm:: classes define hash_value and == for their Hypothesis. 

} // namespace lm
} // namespace feature

#endif
