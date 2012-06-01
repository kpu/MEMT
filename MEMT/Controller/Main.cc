#include "MEMT/Controller/Connection.hh"
#include "MEMT/Controller/CommandLine.hh"
#include "MEMT/Controller/Server.hh"
#include "MEMT/Feature/Length/Process.hh"
#include "MEMT/Feature/LM/Process.hh"
#include "MEMT/Feature/Verbatim/Process.hh"

#include "lm/model.hh"

#include <boost/ptr_container/ptr_vector.hpp>

#include <iostream>
#include <string>

namespace controller {
namespace {

template <class LanguageModel> void RunLoaded(
    const boost::ptr_vector<LanguageModel> &lm,
    const ServiceConfig &config,
    boost::asio::io_service &service,
    boost::asio::ip::tcp::acceptor &acceptor,
    int parent_fd) {
  typedef boost::fusion::vector<
    feature::lm::Process<LanguageModel>,
    feature::verbatim::Process,
    feature::verbatim::Process,
    feature::length::Process> StrategyProcess;
  StrategyProcess process;

  std::vector<const LanguageModel*> model_vec;
  for (size_t i = 0; i < lm.size(); ++i) {
    model_vec.push_back(&lm[i]);
  }
  boost::fusion::at_c<0>(process).SetLM(model_vec);

  // config.port may be 0 for OS assigned port number (the default), so get the
  // actual port here.
  NotifyLoaded(config, acceptor.local_endpoint().port(), parent_fd);

  typedef ConnectionTransition<SentenceTransition<StrategyProcess> > Transition;
  Transition transition(config.connection, process, service);
  RunServer(transition, acceptor);
}

template <class Model> void LoadModels(const std::vector<std::string> &names, boost::ptr_vector<Model> &out) {
  lm::ngram::Config model_config;
  model_config.load_method = util::READ;
  out.clear();
  for (size_t i = 0; i < names.size(); ++i) {
    out.push_back(new Model(names[i].c_str(), model_config));
  }
}

void Load(
    const ServiceConfig &config,
    int parent_fd) {
  using namespace boost::asio::ip;
  // Open the port before the language model is loaded.  
  boost::asio::io_service service;
  tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), config.port));

  lm::ngram::ModelType model_type;
  if (lm::ngram::RecognizeBinary(config.lm.file[0].c_str(), model_type)) {
    switch(model_type) {
      case lm::ngram::HASH_PROBING:
        { 
          boost::ptr_vector<lm::ngram::ProbingModel> models;
          LoadModels<lm::ngram::ProbingModel>(config.lm.file, models);
          RunLoaded(models, config, service, acceptor, parent_fd);
          break;
        }
      case lm::ngram::TRIE_SORTED:
        {
          boost::ptr_vector<lm::ngram::TrieModel> models;
          LoadModels<lm::ngram::TrieModel>(config.lm.file, models);
          RunLoaded(models, config, service, acceptor, parent_fd);
          break;
        }
      case lm::ngram::HASH_SORTED:
      default:
        std::cerr << "Unrecognized kenlm model type " << model_type << std::endl;
        abort();
    }
  } else {
    boost::ptr_vector<lm::ngram::ProbingModel> models;
    LoadModels<lm::ngram::ProbingModel>(config.lm.file, models);
    RunLoaded(models, config, service, acceptor, parent_fd);
  }
}

} // namespace
} // namespace controller

int main(int argc, char *argv[]) {
  controller::ServiceConfig config;
  controller::ParseService(argc, argv, config);

  int parent_fd = config.process.daemonize ? controller::ForkDaemon() : -1;

  controller::Load(config, parent_fd);
}
