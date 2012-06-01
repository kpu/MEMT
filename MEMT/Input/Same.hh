#ifndef _MEMT_Input_Same_h
#define _MEMT_Input_Same_h

namespace input {

class Input;

/* Find sets of words that, when on the frontier at the same time, would
 * produce equal hypotheses.  
 * The words must:
 *   Be aligned via AL_EXACT
 *   Have equal alignments up to type.
 *   Have equal phrase lengths.
 *   Have all words in its phrases the same (recursively according to this definition)
 *
 * Therefore precondition:
 *   Alignments and phrases completed
 */
void FindSame(Input &text);

} // namespace input

#endif
