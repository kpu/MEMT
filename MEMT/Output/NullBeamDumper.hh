/* The decoder's Run function accepts a beam dumper, to which it passes the
 * internal beam of partial hypotheses after each advance. 
 * This one does nothing.
 */

namespace output {
  
struct NullBeamDumper {
  template <class BeamT> void DumpBeam(unsigned int length, const BeamT &beam) {}
};

} // namespace output
