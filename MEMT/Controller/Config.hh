#ifndef _MEMT_Controller_Config_h
#define _MEMT_Controller_Config_h

#include <string>
#include <vector>

namespace controller {
struct SentenceTransitionConfig {
  size_t requests, decoder_workers;
};

struct ConnectionTransitionConfig {
  SentenceTransitionConfig sentence;
  size_t connections;
  size_t output_queue_size;
};

struct LMConfig {
  std::vector<std::string> file;
};

struct ProcessConfig {
  bool daemonize;
  bool keep_open;
  bool call_setsid;
  std::string pidfile, portfile;
};

struct ServiceConfig {
  ConnectionTransitionConfig connection;
  LMConfig lm;
  ProcessConfig process;
  unsigned short int port;
};

} // namespace controller
#endif // _MEMT_Controller_Config_h
