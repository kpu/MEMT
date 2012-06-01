#ifndef _MEMT_Feature_Verbatim_Options_h
#define _MEMT_Feature_Verbatim_Options_h

#include "util/options.hh"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <string>

namespace feature {
namespace verbatim {

class Config;

class VerbatimIndividualExceedsCollective : public util::ArgumentParseError {
  public:
    VerbatimIndividualExceedsCollective(size_t individual, size_t collective);
    virtual ~VerbatimIndividualExceedsCollective() throw() {}
};

class ConfigOptions {
  public:
    explicit ConfigOptions(Config &config, const char *prefix = "score.verbatim");

    void SetDefaults();

    const boost::program_options::options_description &Options() const { return options_; }

    void Finish(const boost::program_options::variables_map &vm, size_t num_systems);

  private:
    Config &config_;

    boost::program_options::options_description options_;
};

} // namespace verbatim
} // namespace feature

#endif // _MEMT_Feature_Verbatim_Options_h
