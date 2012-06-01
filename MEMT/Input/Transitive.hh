#ifndef _MEMT_Input_Transitive_h
#define _MEMT_Input_Transitive_h

namespace input {

class Input;
  
// Returns false if there is a conflict.
bool MakeAlignmentsTransitive(Input &text);

}  // namespace input

#endif
