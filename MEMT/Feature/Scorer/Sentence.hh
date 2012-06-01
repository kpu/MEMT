#ifndef _MEMT_Feature_Scorer_Sentence_h
#define _MEMT_Feature_Scorer_Sentence_h

#include "MEMT/Decoder/Score.hh"
#include "MEMT/Feature/Scorer/Fuzz.hh"
#include "MEMT/Feature/Scorer/Hypothesis.hh"
#include "MEMT/Feature/Length/Sentence.hh"
#include "MEMT/Feature/LM/Sentence.hh"
#include "MEMT/Feature/Verbatim/Sentence.hh"

#include "util/debug.hh"

#include <boost/iterator/zip_iterator.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/view/zip_view.hpp>
#include <boost/mpl/vector.hpp>

#include <math.h>

namespace decoder { class HypHistory; }

namespace feature {
namespace scorer {

namespace detail {

class ResetEach {
  public:
    explicit ResetEach(const input::Input &input) : input_(input) {}
    
    template <class Feature> void operator()(boost::fusion::vector2<Feature &, const typename Feature::Config &> zipped) const {
      boost::fusion::at_c<0>(zipped).Reset(boost::fusion::at_c<1>(zipped), input_);
    }

  private:
    const input::Input &input_;
};

struct BothFeaturesFold {
  typedef size_t result_type;

  template <class Feature> result_type operator()(const result_type previous, const Feature &feature) const {
    return previous + feature.BothFeatures();
  }
};

struct EndFeaturesFold {
  typedef size_t result_type;

  template <class Feature> result_type operator()(const result_type previous, const Feature &feature) const {
    return previous + feature.EndFeatures();
  }
};

// Fold for initializing state.
struct BeginFold {
  typedef LogScore *result_type;

  template <class Feature> result_type operator()(const result_type previous, boost::fusion::vector2<const Feature &, typename Feature::Hypothesis &> act) const {
    const Feature &feature = boost::fusion::at_c<0>(act);
    typename Feature::Hypothesis &state = boost::fusion::at_c<1>(act);
    feature.Begin(state, previous);
    return previous + feature.BothFeatures();
  }
};

// Fold for extending scores by adding a word at location_.
class ExtendFold {
  public:
    // This simultaneously iterates through the previous and appended feature values.  
    typedef boost::zip_iterator<boost::tuple<const LogScore *, LogScore *> > result_type;

    ExtendFold(const input::Input &input, const decoder::HypHistory *history, const input::Location &location)
      : input_(input), history_(history), location_(location) {}

    template <class Feature> result_type operator()(const result_type &previous, boost::fusion::vector3<const Feature &, const typename Feature::Hypothesis &, typename Feature::Hypothesis &> act) const {
      const Feature &feature = boost::fusion::at_c<0>(act);
      const typename Feature::Hypothesis &start_state = boost::fusion::at_c<1>(act);
      typename Feature::Hypothesis &end_state = boost::fusion::at_c<2>(act);
      feature.Extend(input_, history_, location_, start_state, &previous->get<0>(), end_state, &previous->get<1>());
#ifndef NDEBUG
      for (result_type i = previous; i != previous + feature.BothFeatures(); ++i) {
        assert(!isnan(i->get<0>().Log()));
        assert(!isnan(i->get<1>().Log()));
      }
#endif
      return previous + feature.BothFeatures();
    }

  private:
    const input::Input &input_;
    const decoder::HypHistory *const history_;
    const input::Location &location_;
};

class EndFold {
  public:
    typedef LogScore *result_type;

    explicit EndFold(size_t length) : length_(length) {}

    template <class Feature> result_type operator()(const result_type previous, const Feature &feature) const {
      feature.End(length_, previous);
      return previous + feature.EndFeatures();
    }

  private:
    const size_t length_;
};

void RecalculateOverall(const std::vector<LinearScore> &weights, decoder::Score &to_score) {
  LinearScore &log_overall = to_score.MutableOverall().MutableLog();
  log_overall = 0.0;
  std::vector<LinearScore>::const_iterator wi(weights.begin());
  std::vector<LogScore>::const_iterator fi(to_score.Features().begin());
  for (; wi != weights.end(); ++wi, ++fi) {
    assert(!isnan(fi->Log()));
    assert(!isnan(*wi));
    log_overall += fi->Log() * *wi;
    assert(!isnan(log_overall));
  }
}

} // namespace detail

template <class FeaturesT> class Sentence {
  public:
    typedef FeaturesT Features;

    // TODO: remove this.
    typedef typename boost::mpl::front<Features>::type::LanguageModel LanguageModel;

    typedef typename ::feature::scorer::Hypothesis<Features>::type Hypothesis;

    typedef ::feature::scorer::Config Config;

    template <class FeatureInit> explicit Sentence(const FeatureInit &init) : features_(init) {}

    void Reset(const Config &config, const input::Input &input) {
      input_ = &input;

      typedef boost::fusion::vector<Features &, const Config::Features &> Zipme;
      for_each(
          boost::fusion::zip_view<Zipme>(Zipme(features_, config.features)),
          detail::ResetEach(input));
      weights_ = config.weights;
      fuzz_.Reset(config.fuzz_ratio);

      // Fuzz weights.
      fuzz_.Apply(weights_, fuzzed_all_weights_);
      assert(weights_.size() == fuzzed_all_weights_.size());

      size_t both_features_count = fold(features_, size_t(), detail::BothFeaturesFold());
      DEBUG_ONLY(size_t end_features_count = fold(features_, size_t(), detail::EndFeaturesFold()));
      assert(fuzzed_all_weights_.size() == end_features_count + both_features_count);

      fuzzed_both_weights_.clear();
      copy(fuzzed_all_weights_.begin(), fuzzed_all_weights_.begin() + both_features_count, back_inserter(fuzzed_both_weights_));
    }

    void Begin(Hypothesis &state, decoder::Score &scores) const {
      assert(input_);

      // Initialize state and scores.
      scores.MutableFeatures().resize(fuzzed_both_weights_.size());
      typedef boost::fusion::vector<const Features &, Hypothesis &> Zipme;
      fold(
          boost::fusion::zip_view<Zipme>(Zipme(features_, state)),
          &*scores.MutableFeatures().begin(),
          detail::BeginFold());
      detail::RecalculateOverall(fuzzed_both_weights_, scores);
    }

    void Extend(
        const decoder::HypHistory *history,
        const input::Location &location,
        const Hypothesis &from_state,
        const decoder::Score &from_score,
        Hypothesis &to_state,
        decoder::Score &to_score) const {
      assert(from_score.Features().size() == fuzzed_both_weights_.size());
    
      to_score.MutableFeatures().resize(from_score.Features().size());
      const LogScore *from_features = &*from_score.Features().begin();
      LogScore *to_features = &*to_score.MutableFeatures().begin();

      typedef boost::fusion::vector<const Features &, const Hypothesis &, Hypothesis &> Zipme;
      detail::ExtendFold::result_type feature_begin(boost::make_tuple(from_features, to_features));
#ifndef NDEBUG
      detail::ExtendFold::result_type feature_end = 
#endif
      fold(
          boost::fusion::zip_view<Zipme>(Zipme(features_, from_state, to_state)),
          feature_begin,
          detail::ExtendFold(*input_, history, location));
      assert(feature_end == feature_begin + from_score.Features().size());
#ifndef NDEBUG
      for (const LogScore *i = &*to_score.Features().begin(); i != &*to_score.Features().end(); ++i) {
        assert(!isnan(i->Log()));
      }
      for (const double *i = &*fuzzed_both_weights_.begin(); i != &*fuzzed_both_weights_.end(); ++i) {
        assert(!isnan(*i));
      }
#endif
      detail::RecalculateOverall(fuzzed_both_weights_, to_score);
      assert(!isnan(to_score.Overall().Log()));
    }

    void End(
        size_t length,
        bool length_normalize,
        decoder::Score &score,
        std::vector<LogScore> &end_features) const {
      assert(!isnan(score.Overall().Log()));
      if (length_normalize) {
        score.NormalizeByLength(length);
      }
      end_features.resize(fuzzed_all_weights_.size() - fuzzed_both_weights_.size());
      DEBUG_ONLY(LogScore *features_end = )
      fold(
          features_,
          &*end_features.begin(),
          detail::EndFold(length));
      assert(features_end == &*end_features.end());
      
      // Adjust overall accordingly.
      std::vector<LogScore>::const_iterator f_it(end_features.begin());
      std::vector<LinearScore>::const_iterator w_it(fuzzed_all_weights_.begin() + fuzzed_both_weights_.size());
      for (; f_it != end_features.end(); ++f_it, ++w_it) {
        score.MutableOverall().MutableLog() += f_it->Log() * *w_it;
        assert(!isnan(score.MutableOverall().MutableLog()));
      }
    }

  private:
    Features features_;

    const input::Input *input_;

    std::vector<LinearScore> weights_;

    Fuzz fuzz_;

    std::vector<LinearScore> fuzzed_all_weights_;

    std::vector<LinearScore> fuzzed_both_weights_;
};

} // namespace scorer
} // namespace feature

#endif // _MEMT_Feature_Scorer_Sentence_h
