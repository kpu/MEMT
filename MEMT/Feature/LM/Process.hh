#ifndef _MEMT_Feature_LM_Process_h
#define _MEMT_Feature_LM_Process_h

#include "MEMT/Feature/LM/Sentence.hh"

#include <vector>

namespace feature {
namespace lm {

/* TODO: move more lm configuration here. */
template <class LanguageModelT> class Process {
  public:
    typedef LanguageModelT LanguageModel;
    typedef lm::Sentence<LanguageModel> Sentence;

    struct Config {};

    // Workaround for constructing vector of process objects then configuring them
    Process() {}
    void SetLM(const std::vector<const LanguageModel*> &models) { models_ = models; }

    Sentence GetSentence() const { return Sentence(models_); }

    std::vector<unsigned char> Orders() const {
      std::vector<unsigned char> ret;
      for (size_t i = 0; i < models_.size(); ++i) {
        ret.push_back(models_[i]->Order());
      }
      return ret;
    }

  private:
    std::vector<const LanguageModel*> models_;
};

} // namespace lm
} // namespace feature

#endif // _MEMT_Feature_LM_Process_h
