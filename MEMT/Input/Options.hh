#ifndef _MEMT_Input_Options_h
#define _MEMT_Input_Options_h

#include "MEMT/Input/Config.hh"
#include "MEMT/Input/Options.hh"

#include "util/options.hh"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <string>

namespace input {

class ConfigOptions {
  public:
    explicit ConfigOptions(Config &config);

    void SetDefaults();

    const boost::program_options::options_description &Options() const { return options_; }

    void Finish(const boost::program_options::variables_map &vm);

  private:
    Config &config_;

    boost::program_options::options_description options_;

    bool incremental_;
};

} // namespace input

#endif // _MEMT_Input_Options_h
