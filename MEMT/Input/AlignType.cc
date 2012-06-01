#include "MEMT/Input/AlignType.hh"

#include <boost/thread/once.hpp>
#include <boost/unordered_map.hpp>

#include <memory>

namespace input {

const char *kAlignTypeNames[AL_COUNT] = {"unknown", "exact", "snowball_stem", "wn_stem", "wn_synonymy", "paraphrase", "artificial", "self", "transitive", "boundary"};

NotAlignmentTypeName::NotAlignmentTypeName(const StringPiece &name) throw() {
  what_ = "Not an alignment type: ";
  what_.append(name.data(), name.length());
}

namespace {
std::auto_ptr<boost::unordered_map<StringPiece, AlignType> > strings_to_types;

void InitializeAlign() {
  strings_to_types.reset(new boost::unordered_map<StringPiece, AlignType>());
  for (AlignType i = 0; i < AL_COUNT; ++i) {
    (*strings_to_types)[kAlignTypeNames[i]] = 1 << i;
  }
}

boost::once_flag strings_to_types_flag = BOOST_ONCE_INIT;

} // namespace

AlignType TypeFromName(const StringPiece &name) {
  call_once(strings_to_types_flag, InitializeAlign);
  boost::unordered_map<StringPiece, AlignType>::const_iterator i(strings_to_types->find(name));
  if (i == strings_to_types->end()) throw NotAlignmentTypeName(name);
  return i->second;
}

} // namespace input
