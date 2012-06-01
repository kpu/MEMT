#include "MEMT/Strategy/Graph/Options.hh"

#include "MEMT/Strategy/Graph/Config.hh"

#include "util/options.hh"

#include <boost/lexical_cast.hpp>

namespace strategy {
namespace graph {

ConfigOptions::ConfigOptions(Config &config)
  : config_(config), scorer_(config.scorer), options_("Graph strategy"), incremental_(false) {
  namespace po = boost::program_options;

  options_.add(scorer_.Options());

  SetDefaults();
}

void ConfigOptions::SetDefaults() {
  scorer_.SetDefaults();
  incremental_ = false;
}

void ConfigOptions::Finish(const boost::program_options::variables_map &vm, size_t num_systems) {
  scorer_.Finish(vm, num_systems);
}

} // namespace graph
} // namespace strategy
