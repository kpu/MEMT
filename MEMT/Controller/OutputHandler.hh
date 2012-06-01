#ifndef _MEMT_Controller_MEMT_OutputHandler_h
#define _MEMT_Controller_MEMT_OutputHandler_h

#include "MEMT/Controller/CoordWrite.hh"
#include "MEMT/Output/NBest.hh"

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <deque>

namespace controller {

template <class TransitionT> class OutputBackend : boost::noncopyable {
  public:
    typedef TransitionT Transition;
    typedef typename Transition::Request ActualRequest;
    typedef ActualRequest *Request;

    OutputBackend() : counter_(0) {}
    
    void Reset(CoordStream &stream, Transition &transition) {
      base_ = 0;
      assert(requests_.empty());
      requests_.clear();
      stream_ = &stream;
      // should assert(counter_ == 0) but I can't do that with a semaphore.
      transition_ = &transition;
    }

    void operator()(ActualRequest *p_request) {
      ActualRequest &request = *p_request;
      size_t at = request.Sequence() - base_;
      if (at >= requests_.size()) {
        requests_.resize(at + 1);
      }
      requests_[at] = &request;
      while (!requests_.empty() && requests_.front()) {
        ActualRequest &sending = *requests_.front();
        requests_.pop_front();
        ++base_;

        try {
          {
            CoordWrite out(*stream_);
            // This write takes place in parallel with reading from the client and therefore requires Boost >= 1.37.0
            // // See the Asio announcement on http://beta.boost.org/users/news/version_1_37_0
            *out << "nbest" << '\n';
            output::NBest(*out, sending.GetConfig().output, sending.NBest(), sending.Text(), sending.ClientID());
          }
          transition_->Output(sending);
        }
        catch(...) {
          std::cerr << "Writing nbest failed" << std::endl;
          transition_->Failed(sending);
        }
        counter_.post();
      }
    }

    boost::interprocess::interprocess_semaphore &Counter() {
      return counter_;
    }

  private:
    // Sequence number of requests_[0]
    size_t base_;
    // Finished requests waiting to be returned in sequence.
    std::deque<ActualRequest*> requests_;

    CoordStream *stream_;

    boost::interprocess::interprocess_semaphore counter_;

    Transition *transition_;
};

template <class TransitionT> class OutputHandler : boost::noncopyable {
  public:
    typedef TransitionT Transition;
    typedef typename Transition::Request ActualRequest;
    typedef ActualRequest *Request;
    
    explicit OutputHandler(OutputBackend<Transition> &backend) : backend_(backend) {}

    void operator()(ActualRequest *p_request) {
      backend_(p_request);
    }

  private:
    OutputBackend<Transition> &backend_;
};

} // namespace controller

#endif // _MEMT_Controller_MEMT_OutputHandler_h
