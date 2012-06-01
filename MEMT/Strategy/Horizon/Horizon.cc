#include "MEMT/Strategy/Horizon/Horizon.hh"
#include "MEMT/Input/Input.hh"

#include <algorithm>
#include <limits>
#include <vector>

namespace strategy {
namespace horizon {

namespace {

class FromBeginning {
  public:
    FromBeginning(unsigned int length) : length_(length) {}

    unsigned int Start() const {
      return 0;
    }
    bool OffEnd(unsigned int offset) const {
      return offset == length_;
    }
    void Advance(unsigned int &offset) const {
      ++offset;
    }

  private:
    const unsigned int length_;
};

class FromEnd {
  public:
    FromEnd(unsigned int length) : length_(length) {}

    unsigned int Start() const {
      return length_ - 1;
    }
    bool OffEnd(unsigned int offset) const {
      return offset == std::numeric_limits<unsigned int>::max();
    }
    void Advance(unsigned int &offset) const {
      --offset;
    }
  
  private:
    const unsigned int length_;
};

template <class From> void MakeProjection(const input::Input &text, const From &from, unsigned int engine, std::vector<std::vector<int> > &projection) {
  projection.resize(text.engines[engine].words.size());
  // Initialized by BOS or EOS, which are all aligned.
  std::vector<int> closest_shift(text.NumEngines());
  for (unsigned int offset = from.Start(); !from.OffEnd(offset); from.Advance(offset)) {
    const input::Word &word = text.GetWord(engine, offset);
    for (unsigned int other = 0; other < text.engines.size(); ++other) {
      if (word.alignments.Ask(other).IsConnection())
        closest_shift[other] = word.alignments.Ask(other).offset - offset;
    }
    std::vector<int> &project = projection[offset];
    project.clear();
    project.reserve(text.NumEngines());
    for (unsigned int other_engine = 0; other_engine < text.NumEngines(); ++other_engine) {
      project.push_back(closest_shift[other_engine] + offset);
    }
  }
}

unsigned int Bound(unsigned int bottom, unsigned int top, int value) {
  if (static_cast<int>(bottom) >= value) return bottom;
  if (static_cast<int>(top) <= value) return top;
  return static_cast<unsigned int>(value);
}

} // namespace

void System::Reset(const Config &config, const input::Input &input, size_t sys_num) {
  std::vector<std::vector<int> > left_projection, right_projection;
  const size_t num_words = input.engines[sys_num].words.size();
  MakeProjection(input, FromBeginning(num_words), sys_num, left_projection);
  MakeProjection(input, FromEnd(num_words), sys_num, right_projection);

  words_.resize(num_words);

  for (unsigned int offset = 0; offset < num_words; ++offset) {
    Word &word = words_[offset];
    const std::vector<int> &left_project = left_projection[offset];
    const std::vector<int> &right_project = right_projection[offset];
    assert(left_project.size() == right_project.size());
    word.SetNumEngines(left_project.size());
    for (unsigned int other_engine = 0; other_engine < left_project.size(); ++other_engine) {
      unsigned int left = Bound(0, num_words - 1,
          std::min<int>(left_project[other_engine], right_project[other_engine]) - static_cast<int>(config.radius));
      unsigned int right = Bound(0, num_words - 1,
          std::max<int>(left_project[other_engine], right_project[other_engine]) + static_cast<int>(config.radius));
      word.SetBound(other_engine, left, right);
    }
  }
}

void Sentence::Reset(const Config &config, const input::Input &input) {
  config_ = config;

  if (config_.stay_weights.empty()) {
    config_.stay_weights.resize(input.engines.size(), 1.0 / static_cast<LinearScore>(input.engines.size()));
  }
  // Nothing to precompute for length method.
  if (config.method != Config::HORIZON_ALIGNMENT) {
    systems_.clear();
    return;
  }
  systems_.resize(input.engines.size());
  for (size_t s = 0; s < input.engines.size(); ++s) {
    systems_[s].Reset(config, input, s);
  }
}

} // namespace horizon
} // namespace strategy
