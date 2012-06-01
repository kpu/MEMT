#include "MEMT/Strategy/Horizon/Options.hh"

#include "MEMT/Strategy/Horizon/Config.hh"

namespace strategy {
namespace horizon {

std::istream &operator>>(std::istream &i, Config::Method &method) {
  std::string token;
  i >> token;
  if (token == "length") {
    method = strategy::horizon::Config::HORIZON_LENGTH;
  } else if (token == "alignment") {
    method = strategy::horizon::Config::HORIZON_ALIGNMENT;
  } else {
    throw BadHorizonMethod(token);
  }
  return i;
}

BadHorizonMethod::BadHorizonMethod(const std::string &provided) : provided_(provided) {
  what_ = "Bad horizon method: ";
  what_ += provided_;
}

ConfigOptions::ConfigOptions(Config &config)
  : config_(config), options_("Horizon"), incremental_(false) {
  namespace po = boost::program_options;
  options_.add_options()
    ("horizon.stay_weights",
     po::value<std::string>()->notifier(util::MakeSpacedVectorParser(config_.stay_weights)),
     "System weights for purposes of voting on staying or skipping.  Default is uniform.")

    ("horizon.stay_threshold",
     po::value(&config_.stay_threshold),
     "Sum of stay_weights required to stay.")

    ("horizon.radius",
     po::value(&config_.radius),
     "Horizon radius")

    ("horizon.method",
     po::value(&config.method),
     "Horizon method: length or alignment");

  SetDefaults();
}

void ConfigOptions::SetDefaults() {
  config_.stay_threshold = 1.0;
  config_.stay_weights.clear();

  incremental_ = false;
}

void ConfigOptions::Finish(const boost::program_options::variables_map &vm) {
  if (!incremental_) {
    const char *required[] = { "horizon.radius", "horizon.method" };
    util::CheckCount(vm, required);
    incremental_ = true;
  }
}

} // namespace horizon
} // namespace strategy
