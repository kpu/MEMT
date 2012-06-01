#ifndef _MEMT_Controller_Worker_h
#define _MEMT_Controller_Worker_h

#include "util/pcqueue.hh"

#include <boost/noncopyable.hpp>
#include <boost/ref.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>

namespace controller {

template <class RequestT, class HandlerT> class Worker : boost::noncopyable {
  public:
    typedef RequestT Request;
    typedef HandlerT Handler;

    // Externally owned: in and recycle.
    // Takes ownership: handler.
    Worker(util::PCQueue<Request*> &in, util::PCQueue<Request*> &recycle, Handler *handler)
      : in_(in), recycle_(recycle), handler_(handler),
      // This must be the last part of the constructor.  Otherwise there is a race condition.
      thread_(boost::ref(*this)) {}

    // Only call from thread.
    void operator()() {
      Request *req;
      while (in_.Consume(req)) {
        try {
          (*handler_)(*req);
        }
        catch(std::exception &e) {
          std::cerr << "Worker unhandled exception " << e.what() << std::endl;
          recycle_.Produce(req);
        }
        catch(...) {
          std::cerr << "Worker unhandled exception " << std::endl;
          recycle_.Produce(req);
        }
      }
    }

  private:
    util::PCQueue<Request*> &in_, &recycle_;

    boost::scoped_ptr<Handler> handler_;

    boost::thread thread_;
};

} // namespace controller

#endif // _MEMT_Controller_Worker_h
