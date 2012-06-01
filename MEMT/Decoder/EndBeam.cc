#include "MEMT/Decoder/EndBeam.hh"
#include "MEMT/Decoder/History.hh"

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>

#include <algorithm>
#include <queue>
#include <limits>
#include <memory>
#include <vector>

namespace decoder {

namespace {

// Record of path from end of sentence to word.
class BreadCrumb {
  public:
    BreadCrumb(const HypHistory &node, const boost::shared_ptr<BreadCrumb> &right)
      : node_(node), right_(right) {}

    const HypHistory &Node() const {
      return node_;
    }

    const BreadCrumb *Right() const {
      return right_.get();
    }

  private:
    const HypHistory &node_;

    boost::shared_ptr<BreadCrumb> right_;
};

struct HashCrumbByWord : public std::unary_function<const boost::shared_ptr<BreadCrumb> &, size_t> {
  size_t operator()(const boost::shared_ptr<BreadCrumb> &crumb) const {
    size_t ret = 0;
    for (const BreadCrumb *c = crumb.get(); c; c = c->Right()) {
      boost::hash_combine(ret, c->Node().Entry().word);
    }
    return ret;
  }
};

struct EqualCrumbByWord : public std::binary_function<const boost::shared_ptr<BreadCrumb> &, const boost::shared_ptr<BreadCrumb> &, size_t> {
  bool operator()(const boost::shared_ptr<BreadCrumb> &left, const boost::shared_ptr<BreadCrumb> &right) const {
    const BreadCrumb *l = left.get(), *r = right.get();
    while (l != r) {
      if (!l || !r) return false;
      if (l->Node().Entry().word != r->Node().Entry().word) return false;
    }
    return true;
  }
};

HypHistory::const_iterator SecondPlace(const HypHistory &node) {
  HypHistory::const_iterator it(node.begin());
  assert(it != node.end());
  ++it;
  assert(it != node.end());
  return it;
}

HypHistory::const_iterator NextIt(HypHistory::const_iterator i) {
  return ++i;
}

// These go into the queue of paths not taken.
class PartialHistory {
  public:
    // For conversion from EndBeam to queue of PartialHistory only: remember first best.
    // ending.Score() should already be normalized.
    PartialHistory(const EndingHypothesis &ending, bool normalize) :
      packed_(ending.GetHistory()->begin()),
      path_(new BreadCrumb(*ending.GetHistory(), boost::shared_ptr<BreadCrumb>())),
      node_length_(ending.Length() - 1),
      path_length_(1),
      multiplier_(ending.GetScore()),
      score_(ending.GetScore()),
      end_features_(&ending.EndFeatures()) {
      if (normalize) {
        Score normalized_pre(ending.GetHistory()->BestPrevious()->Entry().score);
        normalized_pre.NormalizeByLength(node_length_ + path_length_);
        multiplier_ /= normalized_pre;
      } else {
        multiplier_ /= ending.GetHistory()->BestPrevious()->Entry().score;
      }
    }
      
    // For remembering the second best.  The first best is followed rather than queued.
    PartialHistory(
        const boost::shared_ptr<BreadCrumb> &path,
        const size_t node_length,
        const size_t path_length,
        const Score &best_sentence_score,
        const std::vector<LogScore> &end_features,
        bool normalize) :
      packed_(SecondPlace(path->Node())),
      path_(path),
      node_length_(node_length),
      path_length_(path_length),
      multiplier_(best_sentence_score),
      score_((*packed_)->Entry().score),
      end_features_(&end_features) {
      // Score of first-best.  Divide out by this.  
      const Score &first_previous_score = (*path->Node().begin())->Entry().score;
      if (normalize) {
        Score normalized_previous(first_previous_score);
        normalized_previous.NormalizeByLength(node_length_ + path_length_);
        multiplier_ /= normalized_previous;
        score_.NormalizeByLength(node_length_ + path_length_);
      } else {
        multiplier_ /= first_previous_score;
      }
      score_ *= multiplier_;
      assert(score_.Overall() <= best_sentence_score.Overall() * LogScore(1.0001));
    }

    // Go down to next best thing.
    PartialHistory(const PartialHistory &from, bool normalize) :
      packed_(NextIt(from.packed_)),
      path_(from.path_),
      node_length_(from.node_length_),
      path_length_(from.path_length_),
      multiplier_(from.multiplier_),
      score_((*packed_)->Entry().score),
      end_features_(from.end_features_) {
        
      if (normalize) {
        score_.NormalizeByLength(node_length_ + path_length_);
      }
      score_ *= multiplier_;
    }

    const boost::shared_ptr<BreadCrumb> &Path() const {
      return path_;
    }
    
    // Go down one in packed hypotheses, returning true if there is such a hypothesis.  
    bool HasNextBest() const {
      return Node().end() != NextIt(packed_);
    }

    size_t NodeLength() const { return node_length_; }
    size_t PathLength() const { return path_length_; }

    const Score &GetScore() const { return score_; }

    const HypHistory &Node() const { return path_->Node(); }

    const HypHistory &Packed() const { return **packed_; }

    const std::vector<LogScore> &EndFeatures() const { return *end_features_; }
    
  private:
    // Iterator in Node() of next unused extension.
    HypHistory::const_iterator packed_;
    
    // Path from node_ to EOS, inclusive.
    boost::shared_ptr<BreadCrumb> path_;

    // node_length_ is [BOS to node_).  path_length_ is [node_, EOS].
    size_t node_length_, path_length_;

    // Multiplier by [possibly normalized] score of preceding hypothesis to obtain score_.
    Score multiplier_;

    // Score of best sentence subject to ending with *packed_ and node_ followed by path_.
    Score score_;

    const std::vector<LogScore> *end_features_;
};

bool operator<(const PartialHistory &left, const PartialHistory &right) {
  return (left.GetScore().Overall() == right.GetScore().Overall()) ? 
    (left.NodeLength() > right.NodeLength())
    : (left.GetScore() < right.GetScore());
}

void Completed(std::priority_queue<PartialHistory> &delayed, bool length_normalize, size_t number, std::vector<CompletedHypothesis> &finished) {
  finished.reserve(number);
  finished.clear();

#ifndef NDEBUG
  const LogScore top_overall_norm(delayed.top().GetScore().Overall());
#endif

  // Set of sentences for duplicate removal.
  boost::unordered_set<boost::shared_ptr<BreadCrumb> > sentences;
  while (!delayed.empty()) {
    // Do not make this a reference.  Code here uses delayed.push, which may
    // move top in memory.  Some pushed values have the same score, but lower
    // length, and may therefore become higher priority than top, making
    // pointer recovery impossible.
    const PartialHistory top(delayed.top());
    delayed.pop();
    assert(top.GetScore().Overall() <= top_overall_norm);
    // Enqueue next best.
    if (top.HasNextBest()) {
      delayed.push(PartialHistory(top, length_normalize));
    }
    // Depth first to beginning of sentence, always picking best hypothesis but
    // recording second choice.
    const HypHistory *hist = &top.Packed();
    size_t node_length = top.NodeLength() - 1;
    size_t path_length = top.PathLength() + 1;
    boost::shared_ptr<BreadCrumb> crumb(top.Path());
    for (;
        node_length != std::numeric_limits<typeof(node_length)>::max();
        hist = hist->BestPrevious(), --node_length, ++path_length) {
      assert(hist);
      crumb.reset(new BreadCrumb(*hist, crumb));
      // Save the second choice if any.  
      if (hist->size() > 1) {
        delayed.push(PartialHistory(crumb, node_length, path_length, top.GetScore(), top.EndFeatures(), length_normalize));
      }
    }
    assert(!hist);
    
    // Now crumb encodes a complete sentence.
    if (sentences.insert(crumb).second) {
      // It's not a duplicate either.  
      finished.resize(finished.size() + 1);
      CompletedHypothesis &out = finished.back();
      out.Reset(top.GetScore(), top.EndFeatures());
      for (const BreadCrumb *it = crumb.get(); it; it = it->Right()) {
        const HistoryEntry &ent = it->Node().Entry();
        out.AppendWord(input::Location(ent.engine, ent.offset));
      }
      if (finished.size() == number) return;
    }
  }
}

}  // namespace

void ResetEndBeam(EndBeam &beam, size_t max_size) {
  beam.Reset(max_size);
}

// length_normalize determines if features are normalized for purposes of comparison.  length_feature is the weight of the length feature.
void MakeCompletedHypotheses(const EndBeam &beam, bool length_normalize, std::vector<CompletedHypothesis> &finished) {
  // Queue of branches not immediately considered.
  std::priority_queue<PartialHistory> delayed;
  // Fill delayed with beam contents.
  for (EndBeam::unordered_iterator hyp = beam.unordered_begin(); hyp != beam.unordered_end(); ++hyp) {
    // There should at least be BOS and EOS.
    assert(hyp->Length());
    assert(!hyp->GetHistory()->empty());
#ifndef NDEBUG
      const HypHistory *hist = hyp->GetHistory().get();
      size_t i = 0;
      for (; hist; ++i, hist = hist->BestPrevious()) {}
      assert(i == hyp->Length());
#endif
    delayed.push(PartialHistory(*hyp, length_normalize));
  }

  Completed(delayed, length_normalize, beam.max_size(), finished);
}

} // namespace decoder
