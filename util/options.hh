#ifndef UTIL_OPTIONS__
#define UTIL_OPTIONS__

#include "util/tokenize_piece.hh"

// Argument parsing help for boost::program_options.
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

#include <vector>

namespace util {

class ArgumentParseError : public std::exception {
  public:
    virtual ~ArgumentParseError() throw () {}

    virtual const char *what() const throw() {
      return what_.c_str();
    }

  protected:
    ArgumentParseError(const std::string &what = "Argument parsing error") throw() : what_(what) {}

    std::string what_;
};

class ArgumentCountException : public ArgumentParseError {
  public:
    ArgumentCountException(const char *key, size_t expected, size_t times) throw();

    virtual ~ArgumentCountException() throw() {}

  private:
    std::string key_;
    size_t expected_, times_;
};

void CheckCountRange(const boost::program_options::variables_map &vm, const char **key_begin, const char **key_end, size_t expected = 1);

inline void CheckCount(const boost::program_options::variables_map &vm, const char *key, size_t expected = 1) {
  CheckCountRange(vm, &key, &key + 1, expected);
}

template <size_t size> inline void CheckCount(
    const boost::program_options::variables_map &vm,
    const char *(&keys)[size],
    size_t expected = 1) {
  CheckCountRange(vm, keys, keys + size, expected);
}

template <class T> class SpacedVectorParser {
  public:
    explicit SpacedVectorParser(std::vector<T> &out) : out_(out) {}

    void operator()(const std::string &value) {
      out_.clear();
      for (TokenIter<SingleCharacter, true> i(value, ' '); i; ++i) {
        out_.push_back(boost::lexical_cast<T>(*i));
      }
    }

  private:
   std::vector<T> &out_;
};

template <class T> SpacedVectorParser<T> MakeSpacedVectorParser(std::vector<T> &out) { return SpacedVectorParser<T>(out); }

} // namespace util

#endif // UTIL_OPTIONS__
