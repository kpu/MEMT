#include "MEMT/Feature/LM/Sentence.hh"

#include "MEMT/Input/Input.hh"

namespace feature {
namespace lm {

void LookupVocab(const input::Input &in, const ::lm::base::Vocabulary &vocab, std::vector<std::vector< ::lm::WordIndex> > &indices) {
  indices.resize(in.engines.size());
  for (unsigned int e = 0; e < in.engines.size(); ++e) {
    indices[e].resize(in.engines[e].words.size());
    for (unsigned int o = 0; o < in.engines[e].words.size(); ++o) {
      indices[e][o] = vocab.Index(in.engines[e].words[o].text.Canonical());
    }
  }
}

} // namespace lm
} // namespace feature
