#ifndef _MEMT_Input_Alignment_h
#define _MEMT_Input_Alignment_h

#include "MEMT/Input/AlignType.hh"

#include <assert.h>
#include <map>

#include <boost/scoped_array.hpp>

#include "util/numbers.hh"

namespace input {

// Alignment with just an offset.  This is used as the value of a map from
// engine, so having engine here would be redundant.  
struct AlignmentInfo {
  AlignmentInfo(unsigned int in_offset, AlignType in_type) 
    : offset(in_offset), type(in_type) {}

  AlignmentInfo() : type(0) {}
  
  bool IsNone() const {
    return type == 0;
  }

  bool IsConnection() const {
    return type != 0;
  }

  unsigned int offset;
  AlignType type;
};

// With engine for use outside the vector.
struct Alignment : public AlignmentInfo {
  unsigned int engine;
};

class WordAlignments {
  public:
    WordAlignments() : engines_(0) {}

    WordAlignments(const WordAlignments &from) : alignments_(new AlignmentInfo[from.size()]), engines_(from.size()) {
      CopyAlignments(from);
    }

    WordAlignments &operator=(const WordAlignments &from) {
      alignments_.reset(new AlignmentInfo[from.size()]);
      engines_ = from.size();
      CopyAlignments(from);
      return *this;
    }

    void Reset(size_t engines) {
      if (engines_ != engines) {
        alignments_.reset(new AlignmentInfo[engines]);
        engines_ = engines;
      } else {
        for (size_t i = 0; i < engines_; ++i) {
          alignments_[i].offset = 0;
          alignments_[i].type = 0;
        }
      }
    }

    // Threadsafe if engine numbers differ.
    void Add(unsigned int engine, unsigned int offset, AlignType type);
    
    // From engine to offset and type of alignment.  
    // Once property initialized, a word is always aligned to itself.
    const AlignmentInfo &Ask(unsigned int engine) const {
      return alignments_[engine];
    }

    typedef const AlignmentInfo *const_iterator;
    const_iterator begin() const {
      return alignments_.get();
    }
    const_iterator end() const {
      return alignments_.get() + engines_;
    }

    size_t size() const {
      return engines_;
    }

  private:
    // Paranoid threadsafety: allow parallel access to different parts of this array.
    void CopyAlignments(const WordAlignments &from) {
      for (size_t i = 0; i < from.size(); ++i) {
        alignments_[i] = from.Ask(i);
      }
    }

    boost::scoped_array<AlignmentInfo> alignments_;

    size_t engines_;
};

} // namespace input

#endif
