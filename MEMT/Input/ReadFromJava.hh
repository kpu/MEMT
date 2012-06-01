#ifndef _MEMT_Input_ReadFromJava_h
#define _MEMT_Input_ReadFromJava_h

#include "util/string_piece.hh"

#include <istream>

namespace input {

class Config;

class MatcherFormatError : public std::exception {
  public:
    virtual ~MatcherFormatError() throw() {}

    const char *what() const throw() { return what_.c_str(); }

  protected:
    MatcherFormatError(const std::string &what, const std::string &line) throw();

  private:
    std::string what_;
};

class MatchLineError : public MatcherFormatError {
  public:
    MatchLineError(const std::string &what, size_t top, size_t bottom, const std::string &line) throw();
    virtual ~MatchLineError() throw() {}
};

class TooFewColumns : public MatchLineError {
  public:
    TooFewColumns(size_t i, size_t j, const std::string &line) throw();

    virtual ~TooFewColumns() throw() {}
};

class TooManyColumns : public MatchLineError {
  public:
    TooManyColumns(size_t i, size_t j, const std::string &line) throw();

    virtual ~TooManyColumns() throw() {}
};

class BadColumn : MatchLineError {
  public:
    BadColumn(const StringPiece &entry, size_t i, size_t j, const std::string &line) throw();

    virtual ~BadColumn() throw() {}
};

class AlignmentOffSentenceEnd : public MatcherFormatError {
  public:
    AlignmentOffSentenceEnd(size_t sentence, size_t index, size_t length, const std::string &line) throw();

    virtual ~AlignmentOffSentenceEnd() throw() {}
};

class UnfamiliarAlignmentType : public MatcherFormatError {
  public:
    UnfamiliarAlignmentType(unsigned int type, const std::string &line) throw();

    virtual ~UnfamiliarAlignmentType() throw() {}
};

class SystemNumberDisagreement : public MatcherFormatError {
  public:
    SystemNumberDisagreement(size_t expected, size_t given) throw();

    virtual ~SystemNumberDisagreement() throw() {}
};

class MoreThanCount : public MatcherFormatError {
  public:
    explicit MoreThanCount(const std::string &line) throw();

    virtual ~MoreThanCount() throw() {}
};

class ZeroSystems : public MatcherFormatError {
  public:
    explicit ZeroSystems() throw();

    virtual ~ZeroSystems() throw() {}
};

class Input;

// expected 0 systems means any number will be accepted.
void ReadFromJava(const Config &config, std::istream &in, Input &input, size_t expected = 0);

} // namespace input

#endif // _MEMT_Input_ReadFromJava_h
