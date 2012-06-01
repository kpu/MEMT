#ifndef _MEMT_Controller_ConnectionHandler_h
#define _MEMT_Controller_ConnectionHandler_h

#include "MEMT/Controller/CoordWrite.hh"
#include "MEMT/Controller/Sentence/Config.hh"
#include "MEMT/Controller/Sentence/Options.hh"
#include "MEMT/Controller/OutputHandler.hh"
#include "MEMT/Controller/Worker.hh"
#include "MEMT/Input/Factory.hh"
#include "MEMT/Input/ReadDispatcher.hh"

#include "util/pcqueue.hh"
#include "util/pool.hh"
#include "util/socket_concurrent_iostream.hh"
#include "util/wait_semaphore.hh"

#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/ref.hpp>
#include <boost/utility/in_place_factory.hpp>

namespace controller {

class BadHeaderError : public std::exception {
  public:
    BadHeaderError(const std::string &header) throw() : header_(header) {
      what_ = "Bad header \"";
      what_ += header;
      what_ += "\"";
    }

    ~BadHeaderError() throw() {}

    const char *what() const throw() { return what_.c_str(); }

  private:
    std::string header_, what_;
};

// TransitionT is a SentenceTransition.  This is a connection, so it doesn't
// handle transitions of connections.  
template <class TransitionT> class Connection {
  public:
    typedef TransitionT Transition;
    typedef typename Transition::Request Request;

    Connection(
        std::istream &in, std::ostream &out,
        Transition &transition,
        OutputBackend<Transition> &output_backend,
        util::Pool<OutputHandler<Transition> > &output_pool) :
      sequence_(0),
      in_(in), out_(out),
      transition_(transition),
      output_backend_(output_backend),
      output_pool_(output_pool),
      config_(),
      parser_(config_, transition.LMOrders()) {
      // This should be threadsafe.  
      output_backend_.Reset(boost::ref(out_), boost::ref(transition_));
    }

    ~Connection() {
      try {
        for (size_t i = 0; i < sequence_; ++i) {
          util::WaitSemaphore(output_backend_.Counter());
        }
      }
      catch (...) {
        std::cerr << "Waiting for requests to complete failed" << std::endl;
        abort();
      }
    }

    void Config() {
      ConfigCommand(in_, parser_);
    }

    void FeatureInfo() {
      CoordWrite output(out_);
      size_t feature_count = config_.legacy.scorer.FeatureCount();
      output.Get() << feature_count << std::endl;
      std::vector<feature::base::WeightSign> recommend(feature_count);
      config_.legacy.scorer.WeightHint(&*recommend.begin());
      std::copy(recommend.begin(), recommend.end(), std::ostream_iterator<unsigned>(output.Get(), " "));
      output.Get() << std::endl;
    }

    void Aligned() {
      Request &request = transition_.GetFree();
      try {
        size_t client_id;
        in_ >> client_id;
        request.Reset(config_, sequence_, client_id, output_pool_.In());

        input::ReadDispatcher(config_.input, in_, request.MutableText(), config_.num_systems);

        transition_.Aligned(request);
        ++sequence_;
      }
      catch (...) {
        transition_.Failed(request);
        std::cerr << "Error with matched, entry " << sequence_ << std::endl;
        throw;
       }
    }

    CoordStream &OutputStream() { return out_; }

  private:
    size_t sequence_;

    std::istream &in_;
    CoordStream out_;

    Transition &transition_;
    
    OutputBackend<Transition> &output_backend_;
    util::Pool<OutputHandler<Transition> > &output_pool_;

    sentence::Config config_;
    sentence::ConfigOptions parser_;
};

template <class ConnTransitionT, class SentTransitionT> class ConnectionHandler {
  public:
    typedef ConnTransitionT ConnTransition;
    typedef typename ConnTransition::Request ActualRequest;
    typedef ActualRequest *Request;

    typedef SentTransitionT SentTransition;

    ConnectionHandler(ConnTransition &conn_transition, SentTransition &sent_transition, size_t output_queue_size)
      : conn_transition_(conn_transition), sent_transition_(sent_transition), output_queue_size_(output_queue_size), output_backend_(), output_pool_(output_queue_size, 1, boost::ref(output_backend_), NULL) {}

    bool operator()(ActualRequest *request) {
      if (!request) return false;
      {
        util::ReadSocketStream in(request->GetSocket());
        util::WriteSocketStream out(request->GetSocket());

        Connection<SentTransition> conn(in, out, sent_transition_, output_backend_, output_pool_);

        try {
          std::string header;
          while (in >> header) {
            if (header == "config") {
              conn.Config();
            } else if (header == "matched") {
              conn.Aligned();
            } else if (header == "feature_info") {
              conn.FeatureInfo();
            } else if (header == "bye") {
              std::cerr << "Received bye message." << std::endl;
              break;
            } else {
              throw BadHeaderError(header);
            }
          }
        }
        catch (std::exception &e) {
          std::cerr << e.what() << std::endl;
          CoordWrite writer(conn.OutputStream());
          writer->exceptions(std::ostream::goodbit);
          writer->clear();
          *writer << "error \n" << e.what() << std::endl;
        }
        catch (...) {
          CoordWrite writer(conn.OutputStream());
          writer->exceptions(std::ostream::goodbit);
          writer->clear();
          *writer << "error \nNot std::exception" << std::endl;
        }
      // Wait for requests to finish here.
      }
      std::cerr << "Finished sending replies." << std::endl;
      request->GetSocket().close();
      conn_transition_.Closed(*request);
      return true;
    }

  private:
    ConnTransition &conn_transition_;
    SentTransition &sent_transition_;
    const size_t output_queue_size_;

    OutputBackend<SentTransition> output_backend_;

    // This pool must be of size 1.  Pool is used because it includes the PCQueue.
    util::Pool<OutputHandler<SentTransition> > output_pool_;
};

} // namespace controller

#endif // _MEMT_Controller_ConnectionHandler_h
