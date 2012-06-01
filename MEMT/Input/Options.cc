#include "MEMT/Input/Options.hh"
#include "util/options.hh"

namespace input {

ConfigOptions::ConfigOptions(Config &config)
  : config_(config), options_("Input processing"), incremental_(false) {
  namespace po = boost::program_options;
  options_.add_options()
    ("input.lowercase_before_lm",
     po::value(&config_.lowercase_before_lm),
     "Lowercase input before looking up in LM?")

    ("align.transitive",
     po::value(&config_.transitive),
     "Make alignments transitive?");

  SetDefaults();
}

void ConfigOptions::SetDefaults() {
  config_.lowercase_before_lm = true;
  config_.transitive = false;

  incremental_ = false;
}

void ConfigOptions::Finish(const boost::program_options::variables_map &vm) {
  if (!incremental_) {
    incremental_ = true;
  }
}

} // namespace input
