#ifndef _MEMT_Strategy_Horizon_Options_h
#define _MEMT_Strategy_Horizon_Options_h

#include "util/options.hh"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <string>

namespace strategy {
namespace horizon {

class Config;

class BadHorizonMethod : public util::ArgumentParseError {
  public:
    explicit BadHorizonMethod(const std::string &provided);

    ~BadHorizonMethod() throw() {}

  private:
    std::string provided_;
};

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

} // namespace horizon
} // namespace strategy

#endif // _MEMT_Strategy_Horizon_Options_h
