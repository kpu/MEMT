#include "MEMT/Output/NBest.hh"

#include "MEMT/Decoder/Score.hh"
#include "MEMT/Output/Config.hh"
#include "MEMT/Output/ToString.hh"

#include <fstream>
#include <iostream>
#include <string>

#include <boost/lexical_cast.hpp>

namespace output {
namespace {

void Scores(std::ostream &out, const decoder::CompletedHypothesis &hyp) {
  const decoder::Score &score = hyp.GetScore();
  // Score is the same as final ranking i.e. normalized if the final ranking is.
  for (std::vector<LogScore>::const_iterator f(score.Features().begin()); f != score.Features().end(); ++f) {
    out << f->Log() << ' ';
  }
  for (std::vector<LogScore>::const_iterator f(hyp.EndFeatures().begin()); f != hyp.EndFeatures().end(); ++f) {
    out << f->Log() << ' ';
  }
  out << '\n';
}

void Alignment(std::ostream &out, const decoder::CompletedHypothesis &hyp) {
  assert(hyp.Words().size() >= 2);
  std::vector<input::Location>::const_iterator ending(hyp.Words().end() - 1);
  for (std::vector<input::Location>::const_iterator i = hyp.Words().begin() + 1; i != ending; ++i) {
    out << i->engine << ':' << (i->offset - 1) << ' ';
  }
  out << '\n';
}

void Entry(std::ostream &out, const Config &config, const decoder::CompletedHypothesis &hyp, const input::Input &text) {
  std::string as_string;
  CompletedHypothesisString(config, hyp, text, &as_string);
  out  << as_string << '\n';

  if (config.scores) Scores(out, hyp);
  if (config.alignment) Alignment(out, hyp);
}
} // namespace

void NBest(std::ostream &out, const Config &config, const std::vector<decoder::CompletedHypothesis> &nbest, const input::Input &text, unsigned int sent_id) {
  out << sent_id << ' ' <<  nbest.size() << '\n';
  // Decoder should have extracted at most the requested number.  
  assert(nbest.size() <= config.nbest);
  for (std::vector<decoder::CompletedHypothesis>::const_iterator i = nbest.begin(); i != nbest.end(); ++i) {
    Entry(out, config, *i, text);
  }
  if (config.flush_nbest) {
    out << std::flush;
  }
}

} // namespace output
