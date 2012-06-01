#include "MEMT/Feature/LM/Options.hh"

#include "MEMT/Feature/LM/Config.hh"

namespace feature {
namespace lm {

ConfigOptions::ConfigOptions(Config &config) : config_(config), options_("LM feature") {
  options_.add_options()
    ("score.lm.by_length", boost::program_options::value(&config_.by_length), "Report counts for each n-gram length as a feature?");
}

void ConfigOptions::SetDefaults() {
  config_.by_length = false;
}

void ConfigOptions::Finish(const boost::program_options::variables_map &vm, size_t num_systems, const std::vector<unsigned char> &lm_order) {
  config_.orders = lm_order;
  config_.count = 2 * lm_order.size();
  if (config_.by_length) {
    // Add a feature for all but longest order
    for (std::vector<unsigned char>::const_iterator i = lm_order.begin(); i != lm_order.end(); ++i) {
      config_.count += *i;
    }
    config_.count -= lm_order.size();
  }
}

} // namespace lm
} // namespace feature
