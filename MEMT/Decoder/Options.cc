#include "MEMT/Decoder/Options.hh"

#include "MEMT/Decoder/Config.hh"

#include "util/options.hh"

#include <boost/lexical_cast.hpp>

namespace decoder {

ConfigOptions::ConfigOptions(Config &config)
  : config_(config), options_("Decoding"), incremental_(false) {
  namespace po = boost::program_options;

  options_.add_options()
    ("beam_size",
     po::value(&config_.internal_beam_size),
     "Size of the decoder's internal search beam")

    ("length_normalize",
     po::value(&config_.length_normalize),
     "Length normalize before comparing sentence end scores?");

  SetDefaults();
}

void ConfigOptions::SetDefaults() {
  // Defaults are set here because configuration messages may be updates.
  config_.internal_beam_size = 500;
  config_.end_beam_size = 1;
  config_.length_normalize = true;
  incremental_ = false;
}

void ConfigOptions::Finish(const boost::program_options::variables_map &vm, unsigned int end_beam_size) {
  config_.end_beam_size = end_beam_size;
}

} // namespace decoder
