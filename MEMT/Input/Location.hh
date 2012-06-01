#ifndef _MEMT_Input_Location_h
#define _MEMT_Input_Location_h

namespace input {

struct Location {
  Location() {}
  Location(unsigned int in_engine, unsigned int in_offset) : engine(in_engine), offset(in_offset) {}
  unsigned int engine;
  unsigned int offset;
};

// For sets.
inline bool operator<(const Location &left, const Location &right) {
  if (left.engine < right.engine) return true;
  if (left.engine > right.engine) return false;
  return left.offset < right.offset;
}

} // namespace input

#endif
