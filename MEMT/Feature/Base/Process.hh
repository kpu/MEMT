#ifndef _MEMT_Feature_Base_Process_h
#define _MEMT_Feature_Base_Process_h

namespace feature {
namespace base {

// process-level class for features that don't keep process-level state
template <class SentenceT> class NullProcess {
  public:
    typedef SentenceT Sentence;
    struct Config {};

    NullProcess() {}

    Sentence GetSentence() const {
      return Sentence();
    }
};

} // namespace base
} // namespace feature
#endif // _MEMT_Feature_Base_Process_H
