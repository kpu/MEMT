#include "MEMT/Input/ReadDispatcher.hh"

#include "MEMT/Input/ReadFromJava.hh"

namespace input {

BadFormatName::BadFormatName(const std::string &provided) throw() {
  what_ = "Bad format name ";
  what_ += provided;
}

void ReadDispatcher(const Config &config, std::istream &in, Input &input, size_t expected) {
  std::string format;
  in >> format;
  if (format == "java") {
    ReadFromJava(config, in, input, expected);
  } else {
    throw BadFormatName(format);
  }
}

} // namespace input
