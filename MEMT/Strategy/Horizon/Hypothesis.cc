#include "MEMT/Strategy/Horizon/Hypothesis.hh"

#include "MEMT/Input/Input.hh"
#include "MEMT/Strategy/Phrase/Phrase.hh"
#include "MEMT/Strategy/Horizon/Horizon.hh"

#include<queue>
#include<vector>

using namespace std;

namespace strategy {
namespace horizon {

void Coverage::UseFrontier() {
  vector<bool>::iterator i = find(used_.begin(), used_.end(), false);
  frontier_ += (i - used_.begin()) + 1;
  if (i == used_.end()) {
    used_.clear();
  } else {
    used_.erase(used_.begin(), ++i);
  }
}

void Coverage::Use(unsigned int word) {
  if (word == frontier_) {
    UseFrontier();
  } else if (word > frontier_) {
    unsigned int used_idx = UsedIdx(word);
    if (used_.size() <= used_idx) used_.resize(used_idx + 1);
    used_[used_idx] = true;
  }
}

bool Coverage::UseTo(unsigned int word) {
  if (word <= frontier_) return false;
  unsigned int used_idx = UsedIdx(word);
  if (used_idx >= used_.size()) {
    used_.clear();
    frontier_ = word;
    return true;
  }
  vector<bool>::iterator i = find(used_.begin() + used_idx, used_.end(), false);
  frontier_ += (i - used_.begin()) + 1;
  if (i == used_.end()) {
    used_.clear();
  } else {
    used_.erase(used_.begin(), ++i);
  }
  return true;
}

std::ostream &operator<<(std::ostream &s, const Coverage &coverage) {
  s << "Frontier " << coverage.Frontier() << ":";
  for (std::vector<bool>::const_iterator i = coverage.used_.begin(); i != coverage.used_.end(); ++i) {
    s << *i;
  }
  return s;
}

void Hypothesis::Alignments(const input::Input &text, const strategy::phrase::Sentence &phrase, const input::Location &to) {
  assert(to.offset < text.engines[to.engine].words.size());
  const input::WordAlignments &align = text.GetWord(to.engine, to.offset).alignments;
  for (input::WordAlignments::const_iterator i = align.begin(); i != align.end(); ++i) {
    if ((i-align.begin()) == to.engine) continue;
    if (i->IsNone()) continue;
    const strategy::phrase::PhraseRange &block = phrase[i - align.begin()][i->offset].Block();
    for (unsigned int j = block.start; j <= block.end; ++j) {
      inputs_[i - align.begin()].Use(j);
    }
  }
  inputs_[to.engine].Use(to.offset);
  return;
}

namespace {
  void SkipPunctuation(const input::Engine &engine, Coverage &coverage) {
    while ((coverage.Frontier() < engine.Length())
        && engine.words[coverage.Frontier()].text.IsPunctuation()) {
      coverage.UseFrontier();
    }
  }

  void AdvanceSkipPunctuation(const input::Engine &engine, Coverage &coverage) {
    coverage.UseFrontier();
    SkipPunctuation(engine, coverage);
  }

  struct GreaterFrontierByEngine {
    explicit GreaterFrontierByEngine(const vector<Coverage> &inputs) : inputs_(inputs) {}

    bool operator()(unsigned int left, unsigned int right) const {
      return inputs_[left].Frontier() > inputs_[right].Frontier();
    }
    private:
    const vector<Coverage> &inputs_;
  };

} // namespace

bool Hypothesis::ConsumeWord(const input::Input &text, const Sentence &sentence, const strategy::phrase::Sentence &phrase, const input::Location &location) {
  Alignments(text, phrase, location);
  SkipLeadingPunctuation(text, location.engine);
  if (sentence.GetConfig().method == Config::HORIZON_ALIGNMENT) {
    EnforceNewConstraints(text, sentence);
  } else {
    EnforceOldConstraints(text, sentence);
  }
  for (unsigned int e = 0; e < text.engines.size(); ++e) {
    if (inputs_[e].Frontier() < text.engines[e].Length()) return true;
  }
  return false;
}

void Hypothesis::SkipLeadingPunctuation(const input::Input &text, unsigned int advancing_engine) {
  // Everybody except the engine from which the word was taken skips punctuation.
  for (unsigned int i = 0; i < advancing_engine; ++i)
    SkipPunctuation(text.engines[i], inputs_[i]);
  for (unsigned int i = advancing_engine + 1; i < inputs_.size(); ++i)
    SkipPunctuation(text.engines[i], inputs_[i]);
}

void Hypothesis::EnforceNewConstraints(const input::Input &text, const Sentence &sentence) {
  typedef std::vector<LinearScore> Stay;
  Stay stay(text.engines.size());

  // Calculate minimum sum of used_weights to stay instead of advancing.
  // This is the sum of all used_weights times stay_threshold.
  LinearScore min_stay = sentence.GetConfig().stay_threshold;

  const std::vector<LinearScore> &weights = sentence.GetConfig().stay_weights;

  for (unsigned int e = 0; e < text.engines.size(); ++e) {
    const input::Engine &engine = text.engines[e];
    // TODO: put flag in coverage itself for off end of sentence.
    if (inputs_[e].Frontier() >= engine.Length()) {
      stay[e] = std::numeric_limits<LinearScore>::infinity();
      min_stay -= weights[e];
      if (min_stay < 0.0) return;
      continue;
    }
    const Word &constraints = sentence[e][inputs_[e].Frontier()];
    for (unsigned int o = 0; o < text.engines.size(); ++o) {
      if (constraints.GetBound(o).start <= inputs_[o].Frontier()) {
        stay[o] += weights[e];
      }
    }
  }
  while (1) {  
    Stay::const_iterator lowest = std::min_element(stay.begin(), stay.end());
    if (*lowest >= min_stay) break;
    unsigned int e = lowest - stay.begin();
    const input::Engine &engine = text.engines[e];
    Coverage &coverage = inputs_[e];
    // Subtract.
    {
      const Word &constraints = sentence[e][coverage.Frontier()];
      for (unsigned int o = 0; o < text.engines.size(); ++o)
        if (constraints.GetBound(o).start <= coverage.Frontier())
          stay[o] -= weights[e];
    }
    AdvanceSkipPunctuation(engine, coverage);
    if (coverage.Frontier() >= engine.Length()) {
      stay[e] = std::numeric_limits<LinearScore>::infinity();
      min_stay -= weights[e];
      continue;
    }
    // Don't reference stay[e] here because then the engine will count itself twice.
    LinearScore staying = LinearScore();
    const Word &new_constraints = sentence[e][coverage.Frontier()];
    // Re-add.
    for (unsigned int o = 0; o < text.engines.size(); ++o) {
      const input::Engine &other = text.engines[o];
      if (inputs_[o].Frontier() >= other.Length()) continue;
      if (sentence[o][inputs_[o].Frontier()].GetBound(e).start <= coverage.Frontier())
        staying += weights[o];
      if (new_constraints.GetBound(o).start <= inputs_[o].Frontier())
        stay[o] += weights[e];
    }
    stay[e] = staying;
  }
}

void Hypothesis::EnforceOldConstraints(const input::Input &text, const Sentence &sentence) {
  GreaterFrontierByEngine compare(inputs_);
  priority_queue<unsigned int, vector<unsigned int>, GreaterFrontierByEngine> queue(compare);
  unsigned int max_frontier = 0;
  for (unsigned int i = 0; i < inputs_.size(); ++i) {
    max_frontier = max(max_frontier, inputs_[i].Frontier());
    queue.push(i);
  }
  while(inputs_[queue.top()].Frontier() + sentence.GetConfig().radius < max_frontier) {
    unsigned int below = queue.top(); queue.pop();
    Coverage &coverage = inputs_[below];
    coverage.UseFrontier();
    SkipPunctuation(text.engines[below], coverage);
    // Once we run past end of sentence, ignore it.
    if (coverage.Frontier() >= text.engines[below].Length()) continue;
    // This might skip punctuation on the advancing engine, but only if it skips a word too.
    max_frontier = max(max_frontier, coverage.Frontier());
    queue.push(below);
  }
}

std::ostream &operator<<(std::ostream &s, const Hypothesis &coverage) {
  for (vector<Coverage>::const_iterator i = coverage.inputs_.begin(); i != coverage.inputs_.end(); ++i) {
    s << *i << "\n";
  }
  return s;
}

} // namespace horizon
} // namespace strategy
