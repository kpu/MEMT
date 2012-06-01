#ifndef _MEMT_Controller_Sentence_Options_h
#define _MEMT_Controller_Sentence_Options_h

#include "MEMT/Decoder/Options.hh"
#include "MEMT/Input/Options.hh"
#include "MEMT/Strategy/Legacy/Options.hh"
#include "MEMT/Output/Options.hh"

#include "util/options.hh"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <istream>
#include <string>
#include <vector>

namespace controller {
namespace sentence {

class Config;

class ConfigOptions {
  public:
    explicit ConfigOptions(Config &config, const std::vector<unsigned char> &lm_orders);

    void SetDefaults();

    const boost::program_options::options_description &Options() const { return options_; }

    void Finish(const boost::program_options::variables_map &vm);

  private:
    input::ConfigOptions input_;
    decoder::ConfigOptions decoder_;
    strategy::legacy::ConfigOptions legacy_;
    output::ConfigOptions output_;

    Config &config_;

    boost::program_options::options_description options_;

    bool incremental_;

    std::vector<unsigned char> lm_orders_;
};

void ConfigCommand(std::istream &stream, ConfigOptions &options);

} // namespace sentence
} // namespace controller

#endif // _MEMT_Controller_Sentence_Options_h
