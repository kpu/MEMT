#ifndef _MEMT_Input_ReadDispatcher_h
#define _MEMT_Input_ReadDispatcher_h

#include <istream>
#include <string>

namespace input {

class Config;

class FactoryException : public std::exception {
  public:
    FactoryException() throw() {}
    ~FactoryException() throw() {}

    const char *what() const throw() {
      return "Reading from matcher failed";
    }
};

class BadFormatName : public std::exception {
  public:
    explicit BadFormatName(const std::string &provided) throw();

    ~BadFormatName() throw() {}

    const char *what() const throw() { return what_.c_str(); }

  private:
    std::string what_;
};

class Input;

// Dispatch reading to Perl or Java aligner.
void ReadDispatcher(const Config &config, std::istream &in, Input &input, size_t expected = 0);

} // namespace input

#endif // _MEMT_Input_ReadDispatcher_h
