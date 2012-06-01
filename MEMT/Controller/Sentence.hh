#ifndef _MEMT_Controller_Sentence_h
#define _MEMT_Controller_Sentence_h

#include "MEMT/Controller/Config.hh"
#include "MEMT/Controller/DecoderHandler.hh"
#include "MEMT/Decoder/Completed.hh"
#include "MEMT/Decoder/Config.hh"
#include "MEMT/Input/Config.hh"
#include "MEMT/Input/Input.hh"

#include "util/pcqueue.hh"
#include "util/pool.hh"

#include <boost/ref.hpp>
#include <boost/scoped_array.hpp>
#include <boost/noncopyable.hpp>

#include <vector>

namespace controller {

// A single sentence translation request.  This is what goes through many of the queues.
class SentenceRequest : boost::noncopyable {
  private:
    typedef util::PCQueue<SentenceRequest*> OutputQueueType;

  public:
    SentenceRequest() {}

    // State transitions
    // free to configured.
    void Reset(
        const sentence::Config &config,
        // For output
        size_t sequence, size_t client_id, OutputQueueType &output_queue) {
      config_ = config;
      sequence_ = sequence;
      client_id_ = client_id;
      output_queue_ = &output_queue;

      n_best_.clear();
    }

    size_t Sequence() const { return sequence_; }

    size_t ClientID() const { return client_id_; }

    OutputQueueType &OutputQueue() { return *output_queue_; }

    const sentence::Config &GetConfig() const { return config_; }

    const input::Input &Text() const { return text_; }
    input::Input &MutableText() { return text_; }

    const std::vector<decoder::CompletedHypothesis> &NBest() const { return n_best_; }
    std::vector<decoder::CompletedHypothesis> &MutableNBest() { return n_best_; }

  private:
    sentence::Config config_;

    size_t sequence_, client_id_;
    OutputQueueType *output_queue_;

    input::Input text_;

    std::vector<decoder::CompletedHypothesis> n_best_;
};

template <class StrategyProcessT> class SentenceTransition : boost::noncopyable {
  public:
    typedef StrategyProcessT StrategyProcess;

    typedef SentenceRequest Request;

    SentenceTransition(const SentenceTransitionConfig &config, const StrategyProcess &process)
      : requests_(new Request[config.requests]), free_requests_(config.requests),
        decoder_(config.requests, config.decoder_workers, boost::in_place(boost::ref(*this), boost::ref(process)), NULL),
        // TODO: consult process objects for feature count purposes.  
        lm_orders_(boost::fusion::at_c<0>(process).Orders()) {
      for (size_t i = 0; i < config.requests; ++i) {
        free_requests_.Produce(&requests_[i]);
      }
    }

    const std::vector<unsigned char> &LMOrders() const {
      return lm_orders_;
    }

    Request &GetFree() {
      return *free_requests_.Consume();
    }

    void Aligned(Request &req) {
      decoder_.Produce(&req);
    }

    void Decoded(Request &req) {
      req.OutputQueue().Produce(&req);
    }

    void Output(Request &req) {
      free_requests_.Produce(&req);
    }

    void Failed(Request &req) {
      // TODO: tell the output queue that there was a failure.
      free_requests_.Produce(&req);
    }

  private:
    typedef SentenceTransition<StrategyProcess> Self;

    boost::scoped_array<Request> requests_;
    util::PCQueue<Request*> free_requests_;

    util::Pool<DecoderHandler<Self> > decoder_;

    const std::vector<unsigned char> lm_orders_;
};

} // namespace controller

#endif // _MEMT_Controller_Sentence_h
