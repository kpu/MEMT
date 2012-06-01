#include "MEMT/Alignment/NBest.hh"

#include "MEMT/Alignment/Stemmer.hh"
#include "MEMT/Input/Capitalization.hh"
#include "MEMT/Input/Location.hh"
#include "util/scoped.hh"
#include "util/file_piece.hh"
#include "util/string_piece.hh"
#include "util/tokenize_piece.hh"
#include "util/utf8.hh"

#include <boost/dynamic_bitset.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_map.hpp>

#include <unicode/unistr.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>

namespace input {

typedef unsigned int VertexID;

const VertexID kBOSVertexID = 0;
const VertexID kEOSVertexID = 1;

struct Vertex {
  std::string canonical;
  std::vector<int> cap_votes;
  std::map<VertexID, boost::dynamic_bitset> nexts;
  // Support for first, second, third etc usage
  std::vector<SystemWeights> support;
  // Offsets in each engine.  BOS is 0.  In increasing order.  
  std::vector<std::vector<unsigned int> > offsets;
};

class Aligner {
  public:
    explicit Aligner(const char *language) : stemmer_(language) {}

    void StartSentence(unsigned int num_systems) {
      num_systems_ = num_systems;
      words_.clear();
      vertices_.clear();
      // BOS and EOS vertices
      vertices_.resize(2);
      Vertex &bos = vertices_[kBOSVertexID];
      bos.canonical = "<s>";
      // Votes for lowercase
      bos.cap_votes.resize(3, -1);
      bos.support.resize(1);
      bos.support[0].systems.resize(num_systems, 1.0);
      bos.offsets.resize(num_systems, std::vector<unsigned int>(1, 0));
      Vertex &eos = vertices_[kEOSVertexID];
      eos.canonical = "</s>";
      eos.cap_votes.resize(4, -1);
      eos.support = bos.support;
    }

    void Add(unsigned int system, unsigned int rank, const StringPiece &str) {
      std::pair<std::string, VertexID> to_ins;
      Vertex *previous = &vertices_[kBOSVertexID];
      unsigned int offset = 0;
      for (util::PieceIterator<' '> i(str); i; ++i, ++offset) {
        util::ToLower(*i, to_ins.first);
        to_ins.second = vertices_.size();
        std::pair<boost::unordered_map<std::string, VertexID>::const_iterator, bool> ret(words_.insert(to_ins));
        if (ret.second) {
          vertices_.resize(vertices_.size() + 1);
          vertices_.back().canonical = to_ins.first;
        }
        VertexID id = ret.first->second;
        Vertex &vertex = vertices_[id];
        CollectCaseVotes(*i, offset == 0, vertex.cap_votes);
        SystemWeights &pre_weights = previous->nexts[id];
        if (pre_weights.size() != num_systems_) pre_weights.resize(num_systems_);
        pre_weights[id] += 1.0;
      }
    }

    void Add(unsigned int system, unsigned int segment, NBestReader &reader) {
      StringPiece entry;
      for (unsigned int rank = 0; reader.ReadEntry(segment, entry); ++rank) {
        Add(system, rank, entry);
      }
    }

    void Induce();

  private:
    unsigned int num_systems_;

    boost::unordered_map<std::string, VertexID> words_;

    std::vector<Vertex> vertices_;

    SnowballWrap stemmer_;
};

void Aligner::Induce() {
  for (boost::unordered_map<std::string, std::vector<NBestLocation> >::const_iterator i = words_.begin(); i != words_.end(); ++i) {

  }
}

} // namespace input

int main(int argc, char *argv[]) {
  input::Aligner al("en");
  boost::ptr_vector<input::NBestReader> files;
  for (int i = 1; i < argc; ++i) {
    files.push_back(new input::NBestReader(argv[i]));
  }
  for (unsigned int segment = 0; ; ++segment) {
    bool remaining = false;
    for (boost::ptr_vector<input::NBestReader>::iterator i = files.begin(); i != files.end(); ++i) {
      al.Add(i - files.begin(), segment, *i);
      if (!i->Ended()) remaining = true;
    }
    if (!remaining) break;
  }
}
