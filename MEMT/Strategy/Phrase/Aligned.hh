#ifndef _MEMT_Strategy_Phrase_Aligned_h
#define _MEMT_Strategy_Phrase_Aligned_h

namespace input { class Engine; }

namespace strategy {
namespace phrase {

class System;

void DetectAligned(const input::Engine &engine, System &system);

}  // namespace phrase
}  // namespace strategy

#endif
