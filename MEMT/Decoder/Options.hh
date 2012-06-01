#ifndef _MEMT_Decoder_Options_h
#define _MEMT_Decoder_Options_h

#include "MEMT/Strategy/Horizon/Options.hh"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

namespace decoder {

class Config;

class ConfigOptions {
  public:
    explicit ConfigOptions(Config &config);

    void SetDefaults();

    const boost::program_options::options_description &Options() const { return options_; }

    void Finish(const boost::program_options::variables_map &vm, unsigned int end_beam_size);

  private:
    Config &config_;

    boost::program_options::options_description options_;

    bool incremental_;
};

} // namespace decoder

#endif // _MEMT_Decoder_Options_h
