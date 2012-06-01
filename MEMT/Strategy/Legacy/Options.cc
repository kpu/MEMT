#include "MEMT/Strategy/Legacy/Options.hh"

#include "MEMT/Strategy/Legacy/Config.hh"

#include "util/options.hh"

#include <boost/lexical_cast.hpp>

namespace strategy {
namespace legacy {

ConfigOptions::ConfigOptions(Config &config)
  : config_(config), horizon_(config.horizon), scorer_(config.scorer), options_("Legacy strategy"), incremental_(false) {
  namespace po = boost::program_options;

  options_.add(horizon_.Options());
  options_.add(scorer_.Options());

  options_.add_options()
    ("phrase.type",
     po::value(&config.phrase),
     "Phrase types to use")

    ("legacy.continue_recent",
     po::value(&config.legacy.continue_recent),
     "Allow continuation from the most recently used")

    ("legacy.extend_aligned",
     po::value(&config.legacy.extend_aligned),
     "Extend using aligned words?");

  SetDefaults();
}

void ConfigOptions::SetDefaults() {
  horizon_.SetDefaults();
  scorer_.SetDefaults();
  config_.phrase = strategy::phrase::PHRASE_PUNCTUATION;
  config_.legacy.continue_recent = false;
  config_.legacy.extend_aligned = false;
  incremental_ = false;
}

void ConfigOptions::Finish(const boost::program_options::variables_map &vm, size_t num_systems, const std::vector<unsigned char> &lm_order) {
  horizon_.Finish(vm);
  scorer_.Finish(vm, num_systems, lm_order);
}

} // namespace legacy
} // namespace strategy
