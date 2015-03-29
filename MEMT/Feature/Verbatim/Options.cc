#include "MEMT/Feature/Verbatim/Options.hh"

#include "MEMT/Input/AlignType.hh"
#include "MEMT/Feature/Verbatim/Config.hh"

#include "util/options.hh"
#include "util/tokenize_piece.hh"

#include <boost/lexical_cast.hpp>

namespace feature {
namespace verbatim {
namespace {

class ParseAlignmentMask {
  public:
    explicit ParseAlignmentMask(input::AlignType &out) : out_(out) {}

    void operator()(const std::string &value) {
      out_ = input::TypeFromNames(util::TokenIter<util::SingleCharacter, true>(value, ' '), util::TokenIter<util::SingleCharacter, true>::end());
    }

  private:
    input::AlignType &out_;
};

const char *AppendStr(std::string &temp, const char *first, const char *second) {
  temp.assign(first);
  temp.append(second);
  return temp.c_str();
}

} // namespace

VerbatimIndividualExceedsCollective::VerbatimIndividualExceedsCollective(
    size_t individual,
    size_t collective) {
  what_.assign("Verbatim individual is ");
  what_ += boost::lexical_cast<std::string>(individual);
  what_ += " which is greater than collective ";
  what_ += boost::lexical_cast<std::string>(collective);
}

ConfigOptions::ConfigOptions(Config &config, const char *prefix)
  : config_(config), options_("Verbatim feature") {

  std::string temp;

  namespace po = boost::program_options;

  /* Because temp is reused, these require separate calls to add_options. */
  options_.add_options()
    (AppendStr(temp, prefix, ".individual"),
     po::value(&config_.individual),
     "Maximum n-gram length to report per-system verbatim scores features");

  options_.add_options()
    (AppendStr(temp, prefix, ".collective"),
     po::value(&config_.collective),
     "Maximum n-gram length to report collective (averaged over systems) verbatim features.  collective >= individual.");

  options_.add_options()
    (AppendStr(temp, prefix, ".mask"),
     po::value<std::string>()->notifier(ParseAlignmentMask(config_.mask)),
     "Space separated list of alignment types to include.");

  SetDefaults();
}

void ConfigOptions::SetDefaults() {
  config_.individual = 2;
  config_.collective = 4;
  // Everything but artificial.
  config_.mask = input::AL_UNKNOWN | input::AL_EXACT | input::AL_SNOWBALL_STEM | input::AL_WN_STEM | input::AL_WN_SYNONYMY | input::AL_PARAPHRASE | input::AL_SELF | input::AL_TRANSITIVE | input::AL_BOUNDARY;
}

void ConfigOptions::Finish(const boost::program_options::variables_map &vm, size_t num_systems) {
  if (config_.collective < config_.individual) {
    throw VerbatimIndividualExceedsCollective(config_.individual, config_.collective);
  }
  config_.num_systems = num_systems;
}

} // namespace verbatim
} // namespace feature
