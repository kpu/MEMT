#include "MEMT/Controller/Sentence/Options.hh"

#include "MEMT/Controller/Sentence/Config.hh"

#include "util/bounded_i_stream.hh"
#include "util/options.hh"

#include <boost/scoped_array.hpp>

namespace controller {
namespace sentence {

namespace po = boost::program_options;

ConfigOptions::ConfigOptions(Config &config, const std::vector<unsigned char> &lm_orders)
  : input_(config.input), decoder_(config.decoder), legacy_(config.legacy), output_(config.output),
    config_(config), options_("Sentence level"), incremental_(false), lm_orders_(lm_orders) {
  options_.add_options()
    ("num_systems",
     po::value(&config_.num_systems),
     "Number of systems");

  options_.add(input_.Options()).add(decoder_.Options()).add(legacy_.Options()).add(output_.Options());

  SetDefaults();
}

void ConfigOptions::SetDefaults() {
  input_.SetDefaults();
  decoder_.SetDefaults();
  legacy_.SetDefaults();
  output_.SetDefaults();

  incremental_ = false;
}

class TooFewSystems : public util::ArgumentParseError {
  public:
    TooFewSystems(unsigned int provided) throw() : util::ArgumentParseError(std::string("Too few systems to combine: ") + boost::lexical_cast<std::string>(provided)) {}

    ~TooFewSystems() throw() {}
};

void ConfigOptions::Finish(const boost::program_options::variables_map &vm) {
  if (!incremental_) {
    const char *mandatory_options[] = {"num_systems"};
    util::CheckCount(vm, mandatory_options);
  }
  if (config_.num_systems < 1) throw TooFewSystems(config_.num_systems);
  incremental_ = true;

  input_.Finish(vm);
  output_.Finish(vm);
  decoder_.Finish(vm, config_.output.nbest);
  legacy_.Finish(vm, config_.num_systems, lm_orders_);
}

namespace {

void ParseConfig(std::istream &stream, ConfigOptions &to) {
  po::variables_map vm;
  po::store(po::parse_config_file(stream, to.Options()), vm);
  po::notify(vm);
  to.Finish(vm);
}

class ShortConfigData : std::exception {
  public:
    ShortConfigData() throw () {}

    virtual const char *what() const throw() { return "Configuration data too short."; }
};

} // namespace

void ConfigCommand(std::istream &stream, ConfigOptions &options) {
  std::streamsize length;
  stream >> length;
  util::BoundedIStream limited(stream, length);
  ParseConfig(limited, options);
  if (!limited->Completed()) {
    throw ShortConfigData();
  }
}

} // namespace sentence
} // namespace controller
