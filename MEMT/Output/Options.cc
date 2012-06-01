#include "MEMT/Output/Options.hh"

#include "MEMT/Output/Config.hh"

namespace output {

ConfigOptions::ConfigOptions(Config &config) 
  : config_(config), options_("Output"), incremental_(false) {

  namespace po = boost::program_options;
  options_.add_options()
    ("output.nbest",
     po::value(&config_.nbest),
     "Number of n-best hypotheses")

    ("output.lowercase",
     po::value(&config.lowercase),
     "Lowercase all output?")

    ("output.initial_cap",
     po::value(&config.initial_cap),
     "Capitalize the first word of each output?  No effect if lowercase is true.")

    ("output.scores",
     po::value(&config.scores),
     "Include scores in output?")
    
    ("output.alignment",
     po::value(&config.alignment),
     "Include alignment back to a source hypothesis?")

    ("output.flush_nbest",
     po::value(&config.flush_nbest),
     "Flush after each nbest output?");

  SetDefaults();
}

void ConfigOptions::SetDefaults() {
  config_.nbest = 1;
  config_.lowercase = false;
  config_.initial_cap = true;
  config_.scores = true;
  config_.alignment = false;
  config_.flush_nbest = false;
}

void ConfigOptions::Finish(const boost::program_options::variables_map &vm) {}

} // namespace output
