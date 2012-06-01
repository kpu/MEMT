#include "util/options.hh"

#include <boost/lexical_cast.hpp>

namespace util {

ArgumentCountException::ArgumentCountException(const char *key, size_t expected, size_t times) throw()
  : ArgumentParseError("Expected "), key_(key), expected_(expected), times_(times) {
  what_ += key_;
  what_ += " ";
  what_ += boost::lexical_cast<std::string>(expected);
  what_ += " times, got it ";
  what_ += boost::lexical_cast<std::string>(times);
  what_ += ".";
}

void CheckCountRange(const boost::program_options::variables_map &vm, const char **key_begin, const char **key_end, size_t expected) {
  for (const char **key = key_begin; key != key_end; ++key) {
    if (vm.count(*key) != expected)
      throw ArgumentCountException(*key, expected, vm.count(*key));
  }
}

} // namespace util
