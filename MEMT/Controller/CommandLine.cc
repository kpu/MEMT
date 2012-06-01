#include "MEMT/Controller/CommandLine.hh"

#include "MEMT/Controller/Sentence/Config.hh"
#include "MEMT/Controller/Sentence/Options.hh"

#include "util/options.hh"

#include <boost/program_options.hpp>
#include <boost/thread/thread.hpp>

#include <fstream>
#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <features.h>
#include <unistd.h>

namespace controller {

namespace po = boost::program_options;

void PrintHelp(std::ostream &to, const po::options_description &options) {
  to << "Command line options:" << '\n';
  options.print(to);
  to << "\nConfiguration file options (passed by a client):" << '\n';
  sentence::Config trash;
  sentence::ConfigOptions sentence_options(trash, std::vector<unsigned char>());
  sentence_options.Options().print(to);
}

void ParseService(int argc, char *argv[], ServiceConfig &config) {
  unsigned concurrency = boost::thread::hardware_concurrency();
  if (!concurrency) {
    concurrency = 2;
    std::cerr << "Warning: Boost could not measure hardware parallelism.  Using " << concurrency << " by default.  Override with --controller.decoders." << std::endl;
  }
  po::options_description desc("Server Options");
  desc.add_options()
    ("controller.requests", po::value(&config.connection.sentence.requests)->default_value(50), "Number of open requests")
    ("controller.decoder_workers", po::value(&config.connection.sentence.decoder_workers)->default_value(concurrency), "Number of decoders to run")
    ("controller.connections", po::value(&config.connection.connections)->default_value(4), "Number of open connections")
    ("controller.output_queue_size", po::value(&config.connection.output_queue_size)->default_value(concurrency), "Size of output queue")
    ("lm.file", po::value(&config.lm.file), "File for language model")
    ("daemonize", "Daemonize after printing the \"Accepting connections\" message")
    ("no-setsid", "Do not leave the parent's process tree by calling setsid")
    ("keep-stdio-open", "When daemonizing, keep output open anyway (mostly for debugging).")
    ("pidfile", po::value(&config.process.pidfile), "Write process ID to this file after optionally daemonizing")
    // Work around for boost 1.42.0: port is a prefix of portfile, so it must be defined later.  It's fixed in svn.  
    ("portfile", po::value(&config.process.portfile), "Write port number to this file before daemonizing")
    ("port", po::value(&config.port)->default_value(0), "Port to serve on.  When unspecified, asks the operating system for one.  Retrieve it from the \"Accepting connections\" message")
    ("help", "Print help message");

  if (argc == 1) {
    PrintHelp(std::cerr, desc);
    exit(1);
  }

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    PrintHelp(std::cout, desc);
    exit(0);
  }

  config.process.daemonize = (0 != vm.count("daemonize"));
  config.process.keep_open = (0 != vm.count("keep-stdio-open"));
  config.process.call_setsid = (0 == vm.count("no-setsid"));
  if (!vm.count("lm.file")) throw util::ArgumentCountException("lm.file", 1, 0);
  std::cerr << "Running " << config.connection.sentence.decoder_workers << " decoders." << std::endl;
}

/* Ok this is more complicated than I had hoped for.  The goal is that a script
 * can launch the server, obtain the port number, and have the PID to kill at
 * the end.  This needs to be done without race conditions and preferably
 * without wait loops.  
 * 
 * The port number comes from the command line or the operating system at the
 * time of opening the port.  Therefore, the port number may only be known
 * once open.  The logical thing to do would be open the port, print the
 * number, and daemonize with daemon().  However, some combination of Boost,
 * the OS, and daemon() breaks the TCP server during daemonization.  
 *
 * Here's the process:
 * 1. Do the fork with a fifo back to the parent.  In the child process,
 *    ForkDaemon returns the fd of the pipe to the parent.  
 * 2. Open the TCP server in the child: LoadAndRun
 * 3. Load the language model in the child: also LoadAndRun
 * 4. Complete daemonization and tell the parent the port in NotifyLoaded
 * 5. Parent reads port from fifo, prints to stdout, and exits while still in
 *    ForkDaemon
 */

int ForkDaemon() {
  int pipefd[2];
  if (-1 == pipe(pipefd)) {
    err(1, "open unnamed pipe for daemonize");
  }
  pid_t pid;
  if (-1 == (pid = fork())) {
    err(1, "fork for daemonize");
  }
  if (pid != 0) {
    // Parent.
    if (close(pipefd[1])) err(2, "Could not close write part of pipe in parent");
    char bye = 0;
    if (read(pipefd[0], &bye, 1) == -1)
      err(2, "Child forked to %i but could not read port from it", pid);
    exit(0);
  }
  // Child.
  if (close(pipefd[0])) err(2, "Could not close read part of pipe");
  return pipefd[1];
}

namespace {
template <class T> void OpenWriteClose(const char *name, const T &value) {
  if (!name[0]) {
    return;
  }
  std::ofstream out(name);
  if (!out) {
    warn("Opening %s", name);
    return;
  }
  out << value << std::endl;
  if (!out) {
    warn("Writing %s", name);
    return;
  }
  out.close();
}
} // namespace

void NotifyLoaded(const ServiceConfig &config, unsigned short int port, int parent) {
  OpenWriteClose(config.process.portfile.c_str(), port);
  OpenWriteClose(config.process.pidfile.c_str(), getpid());
  std::cout << "Accepting connections on port " << port << std::endl;
  // Complete daemonization.
  if (config.process.daemonize) {
    if (config.process.call_setsid && (-1 == setsid())) warn("setsid failed");
    if (chdir("/")) warn("Weird, couldn't chdir to /");
    if (!config.process.keep_open) {
      for (int i = 0; i < 3; ++i) {
        if (close(i)) throw std::runtime_error(strerror(errno));
        if (i != open("/dev/null", i ? O_WRONLY|O_APPEND : O_RDONLY))
          throw std::runtime_error(strerror(errno));
      }
    }
    // Tell parent it can die now.  
    const char bye = 0;
    if (-1 == write(parent, &bye, 1))
      warn("Could not write to parent pipe");
    if (close(parent))
      warn("Could not close pipe to parent");
  }
}

} // namespace controller
