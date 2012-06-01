#ifndef _MEMT_Feature_LM_Options_h
#define _MEMT_Feature_LM_Options_h

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <vector>

namespace feature {
namespace lm {

class Config;

class ConfigOptions {
  public:
    explicit ConfigOptions(Config &config);

    void SetDefaults();

    const boost::program_options::options_description &Options() const { return options_; }

    void Finish(const boost::program_options::variables_map &vm, size_t num_systems, const std::vector<unsigned char> &lm_order);

  private:
    Config &config_;

    boost::program_options::options_description options_;
};

} // namespace lm
} // namespace feature

#endif // _MEMT_Feature_LM_Options_h
