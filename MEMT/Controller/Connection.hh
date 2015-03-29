#ifndef _MEMT_Controller_Connection_h
#define _MEMT_Controller_Connection_h

#include "MEMT/Controller/Config.hh"
#include "MEMT/Controller/ConnectionHandler.hh"
#include "MEMT/Controller/Sentence.hh"

#include "util/pcqueue.hh"
#include "util/thread_pool.hh"

#include <boost/asio/ip/tcp.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/noncopyable.hpp>

namespace controller {

class ConnectionRequest : boost::noncopyable {
  public:
    explicit ConnectionRequest(boost::asio::io_service &service)
      : socket_(service) {}

    typedef boost::asio::ip::tcp::socket Socket;
    Socket &GetSocket() { return socket_; }

  private:
    Socket socket_;
};

template <class SentTransitionT> class ConnectionTransition : boost::noncopyable {
  public:
    typedef SentTransitionT SentTransition;
    typedef typename SentTransition::StrategyProcess StrategyProcess;
    typedef ConnectionRequest Request;

    ConnectionTransition(const ConnectionTransitionConfig &config, const StrategyProcess &process, boost::asio::io_service &service)
      : free_requests_(config.connections),
        sent_transition_(config.sentence, process),
        connection_(config.connections, config.connections, boost::in_place(boost::ref(*this), boost::ref(sent_transition_), config.output_queue_size), NULL) {
      for (size_t i = 0; i < config.connections; ++i) {
        requests_.push_back(new ConnectionRequest(service));
        free_requests_.Produce(&requests_.back());
      }
    }

    Request &GetFree() {
      return *free_requests_.Consume();
    }

    void Opened(Request &req) {
      connection_.Produce(&req);
    }

    void Closed(Request &req) {
      CloseAndFree(req);
    }

    void Failed(Request &req) {
      CloseAndFree(req);
    }

  private:
    void CloseAndFree(Request &req) {
      try {
        req.GetSocket().close();
      }
      catch(...) {
        std::cerr << "Exception while closing a socket." << std::endl;
        abort();
      }
      free_requests_.Produce(&req);

    }

    typedef ConnectionTransition<SentTransitionT> Self;

    boost::ptr_vector<Request> requests_;
    util::PCQueue<Request*> free_requests_;

    SentTransition sent_transition_;

    util::ThreadPool<ConnectionHandler<Self, SentTransitionT> > connection_;
};

} // namespace controller

#endif // _MEMT_Controller_Connection_h
