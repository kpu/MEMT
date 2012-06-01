#ifndef _MEMT_Feature_Scorer_Options_h
#define _MEMT_Feature_Scorer_Options_h

#include "MEMT/Feature/LM/Options.hh"
#include "MEMT/Feature/Verbatim/Options.hh"

#include "util/numbers.hh"
#include "util/options.hh"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <string>

namespace feature {
namespace scorer {

class Config;

class WeightCountMismatch : public util::ArgumentParseError {
  public:
    WeightCountMismatch(size_t expected, size_t provided);
    virtual ~WeightCountMismatch() throw() {}
};

class ConfigOptions {
  public:
    explicit ConfigOptions(Config &config);

    void SetDefaults();

    const boost::program_options::options_description &Options() const { return options_; }

    void Finish(const boost::program_options::variables_map &vm, size_t num_systems, const std::vector<unsigned char> &lm_order);

  private:
    lm::ConfigOptions lm_;
    verbatim::ConfigOptions verbatim0_, verbatim1_;

    Config &config_;

    boost::program_options::options_description options_;

    bool incremental_;

    std::string weight_string_;
};

} // namespace scorer
} // namespace feature

#endif // _MEMT_Feature_Scorer_Options_h
