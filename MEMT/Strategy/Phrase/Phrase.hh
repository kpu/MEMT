#ifndef _MEMT_Strategy_Phrase_Phrase_h
#define _MEMT_Strategy_Phrase_Phrase_h

#include "MEMT/Strategy/Phrase/Type.hh"

#include <algorithm>
#include <map>
#include <ostream>
#include <vector>

namespace input { class Engine; class Input; }

namespace strategy {
namespace phrase {

// An inclusive range of offsets to block off if this word is chosen as part of an alignment.
struct PhraseRange {
  unsigned int start;
  unsigned int end;
};

struct Phrase : public PhraseRange {
  Phrase() : type(0) {}
  Type type;
};

class System;

class Word {
  public:
    typedef std::map<unsigned int, Type> EndMap;

    Word() {}

    const EndMap &Ends() const { return ends_; }

    const PhraseRange &Block() const { return block_; }

  protected:
    friend class System;
    friend void DetectAligned(const input::Engine &engine, System &system);
    friend void DetectPunctuation(const input::Engine &engine, System &system);
    friend void ComputeBlocks(System &system);

    void Reset(unsigned int offset) {
      ends_.clear();
      block_.start = offset;
      block_.end = offset;
    }

    // Ends are where phrases that start here end.
    void AddEnd(unsigned int end, Type type) {
      ends_[end] |= type;
    }

    /* Blocks are offsets within the same engine that get blocked off if this
     * word is used as the result of an alignment.  Currently, this is the entire
     * Punctuation or SourceChunk phrase if we appear in one.
     */
    void AddBlock(const PhraseRange &blockage) {
      block_.start = std::min(blockage.start, block_.start);
      block_.end = std::max(blockage.end, block_.end);
    }

  private:
    PhraseRange block_;

    // Key is end of phrase, inclusive.  Value is type of phrase.
    EndMap ends_;
};

std::ostream &operator<<(std::ostream &str, const Word &phrases);

class System {
  public:
    System() {}

    const Word &operator[](size_t w) const { return words_[w]; }

    size_t size() const { return words_.size(); }

  protected:
    friend class Sentence;
    friend void DetectAligned(const input::Engine &engine, System &system);
    friend void DetectPunctuation(const input::Engine &engine, System &system);
    friend void ComputeBlocks(System &system);

    void Reset(Type types, const input::Engine &engine);

    Word &operator[](size_t w) { return words_[w]; }

  private:
    std::vector<Word> words_;
};

class Sentence {
  public:
    Sentence() {}

    void Reset(Type type, const input::Input &input);

    const System &operator[](size_t s) const { return systems_[s]; }

    size_t size() const { return systems_.size(); }

  private:
    std::vector<System> systems_;
};

} // namespace phrase
} // namespace strategy

#endif // _MEMT_Strategy_Phrase_Phrase_h
