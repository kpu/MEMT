#include "MEMT/Strategy/Phrase/Punctuation.hh"

#include "MEMT/Input/Input.hh"
#include "MEMT/Strategy/Phrase/Phrase.hh"

#include <assert.h>

namespace strategy {
namespace phrase {

void DetectPunctuation(const input::Engine &engine, System &system) {
  //A punctuation phrase is the punctuation mark itself and the word
  //before it, except when it's the first word of the sentence:
  size_t start = 0;
  bool open = false;

  // <EOS> is not punctuation.  Therefore we will always close.
  for (unsigned i = 1; i < engine.Length(); ++i) {
    if (engine.words[i].text.IsPunctuation()) {
      if (!open) {
        open = true;
        start = i - 1;
      }
    } else if (open) {
      system[start].AddEnd(i - 1, PHRASE_PUNCTUATION);
      open = false;
    }
  }
  assert(!open);
}

}  // namespace phrase
}  // namespace strategy
