#include "MEMT/Feature/Scorer/Options.hh"

#include "MEMT/Feature/Scorer/Config.hh"

#include "util/options.hh"
#include "util/tokenize_piece.hh"

#include <boost/fusion/algorithm.hpp>
#include <boost/lexical_cast.hpp>

namespace feature {
namespace scorer {

namespace detail {
struct FeatureCountFold {
  typedef size_t result_type;

  template <class Feature> result_type operator()(const result_type previous, const Feature &feature) const {
    return previous + feature.FeatureCount();
  }
};
} // namespace detail

WeightCountMismatch::WeightCountMismatch(
    size_t expected,
    size_t provided) {
  what_ = std::string("Expected ");
  what_ += boost::lexical_cast<std::string>(expected);
  what_ += " weights but ";
  what_ += boost::lexical_cast<std::string>(provided);
  what_ += " weights were provided.";
}

ConfigOptions::ConfigOptions(Config &config)
  : lm_(boost::fusion::at_c<0>(config.features)), verbatim0_(boost::fusion::at_c<1>(config.features), "score.verbatim0"), verbatim1_(boost::fusion::at_c<2>(config.features), "score.verbatim1"), /*cooccur_(boost::fusion::at_c<3>(config.features)),*/ config_(config), options_("Scorer"), incremental_(false) {
    namespace po = boost::program_options;

    options_.add(lm_.Options());
    options_.add(verbatim0_.Options());
    options_.add(verbatim1_.Options());

    options_.add_options()
      ("score.weights",
       po::value(&weight_string_),
       "Scoring weights as a vector in the same order as the n-best list")

      ("score.fuzz.ratio",
       po::value(&config_.fuzz_ratio),
       "Proportion of scoring weight to randomly fuzz.  Useful for seeding MERT.");

    SetDefaults();
  }

void ConfigOptions::SetDefaults() {
  lm_.SetDefaults();
  verbatim0_.SetDefaults();
  verbatim1_.SetDefaults();
  // Defaults are set here because configuration messages may be updates.
  config_.fuzz_ratio = 0.0;
  incremental_ = false;
}

void ConfigOptions::Finish(const boost::program_options::variables_map &vm, size_t num_systems, const std::vector<unsigned char> &lm_order) {
  lm_.Finish(vm, num_systems, lm_order);
  verbatim0_.Finish(vm, num_systems);
  verbatim1_.Finish(vm, num_systems);

  if (!incremental_) {
    util::CheckCount(vm, "score.weights");
    incremental_ = true;
  }

  const size_t expected_weights = fold(config_.features, 0, detail::FeatureCountFold());
  if (vm.count("score.weights")) {
    config_.weights.clear();
    if (weight_string_ == "uniform") {
      config_.weights.resize(expected_weights, 1.0);
    } else if (weight_string_ == "zero") {
      config_.weights.resize(expected_weights, 0.0);
    } else {
      for (util::PieceIterator<' '> i(weight_string_); i; ++i) {
        config_.weights.push_back(boost::lexical_cast<LinearScore>(*i));
      }
    }
  }
  if (expected_weights != config_.weights.size()) throw WeightCountMismatch(expected_weights, config_.weights.size());
}

} // namespace scorer
} // namespace feature
