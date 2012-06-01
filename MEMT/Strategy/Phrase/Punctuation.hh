#ifndef _MEMT_Strategy_Phrase_Punctuation_h
#define _MEMT_Strategy_Phrase_Punctuation_h

namespace input { class Engine; }

namespace strategy {
namespace phrase {

class System;
void DetectPunctuation(const input::Engine &engine, System &system);

}  // namespace phrase
}  // namespace strategy

#endif
