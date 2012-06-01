#ifndef _MEMT_Controller_Server_h
#define _MEMT_Controller_Server_h

#include <boost/asio/ip/tcp.hpp>

#include <fstream>

#include <err.h>
#include <sys/types.h>
#include <unistd.h>

namespace controller {

template <class ConnTransition> void RunServer(ConnTransition &transition, boost::asio::ip::tcp::acceptor &acceptor) {
  using namespace boost::asio::ip;

  while (1) {
    ConnectionRequest *req = NULL;
    try {
      // Get a socket and make sure it's clear.
      req = &transition.GetFree();
      tcp::socket &socket = req->GetSocket();
      socket.close();

      acceptor.accept(socket);
      std::cerr << "Got connection." << std::endl;
      transition.Opened(*req);
    }
    catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      if (req) transition.Failed(*req);
    }
    catch (...) {
      std::cerr << "Some server exception" << std::endl;
      if (req) transition.Failed(*req);
    }
  }
}

} // namespace controller

#endif // _MEMT_Controller_Server_h
