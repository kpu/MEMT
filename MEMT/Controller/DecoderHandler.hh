#ifndef _MEMT_Controller_DecoderHandler_h
#define _MEMT_Controller_DecoderHandler_h

#include "MEMT/Input/Factory.hh"
#include "MEMT/Decoder/Implementation.hh"
#include "MEMT/Output/NullBeamDumper.hh"
#include "MEMT/Output/StderrBeamDumper.hh"
#include "MEMT/Strategy/Legacy/Sentence.hh"

#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/sequence/intrinsic/value_at.hpp>
#include <boost/ref.hpp>

namespace controller {

namespace detail {
template <class ProcessT> struct SentenceOp {
  typedef typename ProcessT::Sentence type;
};

struct MakeSentenceOp {
  template <class T> struct result;
  template <class Process> struct result<MakeSentenceOp(const Process &)> {
    typedef typename Process::Sentence type;
  };
  template <class Process> typename Process::Sentence operator()(const Process &proc) const {
    return proc.GetSentence();
  }
};
} // namespace detail

template <class TransitionT> class DecoderHandler : boost::noncopyable {
  public:
    typedef TransitionT Transition;

    typedef typename Transition::Request ActualRequest;
    // This is so util::PCQueue's pool uses references instead of copies.
    typedef ActualRequest *Request;

    typedef typename Transition::StrategyProcess StrategyProcess;

    // TODO: move vocab part fully into features.
    explicit DecoderHandler(Transition &transition, const StrategyProcess &process)
      : transition_(transition), strategy_(boost::fusion::transform(process, detail::MakeSentenceOp())) {}

    void operator()(ActualRequest *request) {
      try {
        output::NullBeamDumper dumper;
        input::ProcessAligned(request->GetConfig().input, request->MutableText());
        strategy_.Reset(request->GetConfig().legacy, request->Text());
        decoder_.Run(request->GetConfig().decoder, request->Text(), strategy_, dumper, request->MutableNBest());
        transition_.Decoded(*request);
      }
      catch(std::exception &e) {
        std::cerr << "Decoder exception " << e.what() << std::endl;
        abort();
      }
      catch(...) {
        std::cerr << "Decoder exception" << std::endl;
        abort();
      }
    }

  private:
    Transition &transition_;

    typedef typename boost::fusion::result_of::value_at_c<StrategyProcess, 0>::type::LanguageModel LanguageModel;
    typedef typename LanguageModel::Vocabulary Vocabulary;

    typedef typename boost::mpl::transform<StrategyProcess, detail::SentenceOp<boost::mpl::_1> >::type Features;
    //typedef boost::fusion::vector<feature::lm::Sentence<LanguageModel>, feature::verbatim::Sentence, feature::verbatim::Sentence, feature::length::Sentence, feature::cooccur::Sentence> Features;
    typedef strategy::legacy::Sentence<Features> Strategy;

    Strategy strategy_;

    decoder::DecoderImpl<Strategy> decoder_;
};

} // namespace controller

#endif // _MEMT_Controller_DecoderHandler_h

