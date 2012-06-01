#ifndef _MEMT_Controller_CommandLine_h
#define _MEMT_Controller_CommandLine_h

#include "MEMT/Controller/Config.hh"

#include "util/options.hh"

#include <string>

namespace controller {

void ParseService(int argc, char *argv[], ServiceConfig &config);

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

int ForkDaemon();

void NotifyLoaded(const ServiceConfig &config, unsigned short int port, int parent);

} // namespace controller

#endif // _MEMT_Controller_CommandLine_h
