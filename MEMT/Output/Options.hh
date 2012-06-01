#ifndef _MEMT_Output_Options_h
#define _MEMT_Output_Options_h

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

namespace output {

class Config;

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

} // namespace output

#endif // _MEMT_Output_Options_h
