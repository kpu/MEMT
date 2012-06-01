#include "MEMT/Feature/Verbatim/Sentence.hh"

#include "MEMT/Decoder/History.hh"
#include "MEMT/Input/Input.hh"
#include "MEMT/Feature/Verbatim/Config.hh"

#include <algorithm>

namespace feature {
namespace verbatim {

void Sentence::Begin(Hypothesis &start_state, LogScore *start_scores) const {
  start_state.clear();
  // BOS counts for 1 because it's aligned.  This doesn't matter anyway because it amounts to a multiplicative factor on the score.  
  start_state.resize(config_.num_systems, 1);
  LogScore *end = start_scores + BothFeatures();
  for (; start_scores != end; ++start_scores) {
    start_scores->SetLog(0.0);
  }
}

void Sentence::Extend(
    const input::Input &input,
    const decoder::HypHistory *history,
    const input::Location &append,
    const Hypothesis &from_state,
    const LogScore *from_scores,
    Hypothesis &to_state,
    LogScore *to_scores) const {
  assert(from_state.size() == config_.num_systems);
  to_state.resize(config_.num_systems);

  assert(history);
  const input::WordAlignments &previous = input.GetWord(history->Entry().engine, history->Entry().offset).alignments;
  const input::WordAlignments &current = input.GetWord(append).alignments;

  for (size_t e = 0; e < config_.num_systems; ++e) {
    const input::AlignmentInfo &pre = previous.Ask(e);
    const input::AlignmentInfo &cur = current.Ask(e);
    // Determine the length of consecutive monotone alignment.
    if ((pre.type & config_.mask) && (cur.type & config_.mask) && (pre.offset + 1 == cur.offset)) {
      to_state[e] = from_state[e] + 1;
    } else if (cur.type & config_.mask) {
      to_state[e] = 1;
    } else {
      to_state[e] = 0;
    }
  }
    
  const LogScore *from_iter = from_scores;
  LogScore *to_iter = to_scores;
  // Compute individual scores.
  for (size_t e = 0; e < config_.num_systems; ++e) {
    // Add 1 to each length match feature up to individual
    LogScore *add_1_end = to_iter + std::min(to_state[e], config_.individual);
    LogScore *add_0_end = to_iter + config_.individual;
    for (; to_iter != add_1_end; ++to_iter, ++from_iter) {
      to_iter->SetLog(from_iter->Log() + 1.0);
    }
    // Copy the rest.
    for (; to_iter != add_0_end; ++to_iter, ++from_iter) {
      *to_iter = *from_iter;
    }
  }

  assert(from_iter == from_scores + config_.num_systems * config_.individual);
  assert(to_iter == to_scores + config_.num_systems * config_.individual);

  // Compute collective scores.
  assert(config_.collective >= config_.individual);
  LogScore *collective_to_start = to_iter;
  std::copy(from_iter, from_iter + config_.collective - config_.individual, collective_to_start);
  LogScore *collective_to_0idx = collective_to_start - config_.individual;
  for (size_t e = 0; e < config_.num_systems; ++e) {
    if (to_state[e] < config_.individual) continue;
    for (LogScore *i = collective_to_start; i != collective_to_0idx + std::min(to_state[e], config_.collective); ++i) {
      i->MutableLog() += 1.0;
    }
  }
}

} // namespace verbatim
} // namespace feature
