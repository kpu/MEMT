#ifndef _MEMT_Strategy_Graph_Options_h
#define _MEMT_Strategy_Graph_Options_h

#include "MEMT/Strategy/Scorer/Options.hh"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

namespace strategy {
namespace graph {

class Config;

class ConfigOptions {
  public:
    explicit ConfigOptions(Config &config);

    void SetDefaults();

    const boost::program_options::options_description &Options() const { return options_; }

    void Finish(const boost::program_options::variables_map &vm, size_t num_systems);

  private:
    Config &config_;

    scorer::ConfigOptions scorer_;    

    boost::program_options::options_description options_;

    bool incremental_;
};

} // namespace graph
} // namespace decoder

#endif // _MEMT_Strategy_Graph_Options_h
