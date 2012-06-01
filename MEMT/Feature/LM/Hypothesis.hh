#ifndef _MEMT_Feature_LM_Hypothesis_h
#define _MEMT_Feature_LM_Hypothesis_h

namespace feature {
namespace lm {

template <class LanguageModel> struct Hypothesis {
  typedef typename LanguageModel::State T;
};

} // namespace lm
} // namespace feature

#endif // _MEMT_Feature_LM_Hypothesis_h
