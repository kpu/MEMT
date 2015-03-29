#include "MEMT/Input/ReadFromJava.hh"

#include "MEMT/Input/Alignment.hh"
#include "MEMT/Input/Input.hh"
#include "MEMT/Input/Read.hh"
#include "util/tokenize_piece.hh"

#include <boost/lexical_cast.hpp>

#include <istream>
#include <string>

namespace input {

class Config;

MatcherFormatError::MatcherFormatError(const std::string &what, const std::string &line) throw()
  : what_(what + " in line '" + line + "'") {}

MatchLineError::MatchLineError(const std::string &what, size_t top, size_t bottom, const std::string &line) throw() 
  : MatcherFormatError(what + " in alignment between sentences " + boost::lexical_cast<std::string>(top) + " and " + boost::lexical_cast<std::string>(bottom), line) {}

TooFewColumns::TooFewColumns(size_t i, size_t j, const std::string &line) throw()
  : MatchLineError("Too few columns", i, j, line) {}

TooManyColumns::TooManyColumns(size_t i, size_t j, const std::string &line) throw()
  : MatchLineError("Too many columns", i, j, line) {}

BadColumn::BadColumn(const StringPiece &entry, size_t i, size_t j, const std::string &line) throw()
  : MatchLineError(std::string("Bad column '") + boost::lexical_cast<std::string>(entry) + "'", i, j, line) {}

AlignmentOffSentenceEnd::AlignmentOffSentenceEnd(size_t sentence, size_t index, size_t length, const std::string &line) throw()
  : MatcherFormatError(std::string("Alignment to position ") + boost::lexical_cast<std::string>(index) + " in sentence " + boost::lexical_cast<std::string>(sentence) + " which has length " + boost::lexical_cast<std::string>(length), line) {}

UnfamiliarAlignmentType::UnfamiliarAlignmentType(unsigned int type, const std::string &line) throw()
  : MatcherFormatError(std::string("Unfamiliar alignment type ") + boost::lexical_cast<std::string>(type), line) {}

SystemNumberDisagreement::SystemNumberDisagreement(size_t expected, size_t provided) throw()
  : MatcherFormatError(std::string("Expected ") + boost::lexical_cast<std::string>(expected) + " systems but got " + boost::lexical_cast<std::string>(provided), boost::lexical_cast<std::string>(provided)) {}

MoreThanCount::MoreThanCount(const std::string &line) throw()
  : MatcherFormatError(std::string("Expected just a count"), line) {}

ZeroSystems::ZeroSystems() throw() : MatcherFormatError("Zero systems specified", "0") {}

namespace {

static const AlignType kAlignJavaMap[] = {AL_EXACT, AL_SNOWBALL_STEM, AL_WN_SYNONYMY, AL_PARAPHRASE};

// convert *fields to Ret.  Use line for error reporting purposes.
template <class Ret> Ret Munch(util::TokenIter<util::SingleCharacter, false> &fields, size_t i, size_t j, const std::string &line) {
  if (!fields) throw TooFewColumns(i, j, line);
  Ret ret;
  try {
    ret = boost::lexical_cast<Ret>(*fields);
  }
  catch (boost::bad_lexical_cast &e) {
    throw BadColumn(*fields, i, j, line);
  }
  ++fields;
  return ret;
}

} // namespace

void ReadFromJava(const Config &config, std::istream &in, Input &input, size_t expected) {
  in.exceptions(std::istream::eofbit | std::istream::failbit | std::istream::badbit);
  size_t num;
  in >> num;
  if (expected && (num != expected))
    throw SystemNumberDisagreement(expected, num);
  if (!num) throw ZeroSystems();
  input.SetupEngines(num);
  std::string line;
  getline(in, line);
  if (!line.empty()) throw MoreThanCount(line);
  ReadAllEngines(config, in, input);

  for (size_t i = 0; i < num; ++i) {
    for (size_t j = i + 1; j < num; ++j) {
      while (getline(in, line) && (!line.empty())) {
        util::TokenIter<util::SingleCharacter, false> fields(line, '\t');
        size_t first = Munch<size_t>(fields, i, j, line);
        size_t second = Munch<size_t>(fields, i, j, line);
        unsigned int type_int = Munch<unsigned int>(fields, i, j, line);
        Munch<float>(fields, i, j, line);
        if (fields) throw TooManyColumns(i, j, line);

        // Skip over BOS and EOS
        ++first;
        ++second;

        if (first >= input.engines[i].words.size()) throw AlignmentOffSentenceEnd(i, first, input.engines[i].words.size(), line);
        if (second >= input.engines[j].words.size()) throw AlignmentOffSentenceEnd(j, second, input.engines[j].words.size(), line);

        Word &first_word = input.engines[i].words[first];
        Word &second_word = input.engines[j].words[second];

        if (type_int >= sizeof(kAlignJavaMap) / sizeof(AlignType)) throw UnfamiliarAlignmentType(type_int, line);
        AlignType type = kAlignJavaMap[type_int];

        first_word.alignments.Add(j, second, type);
        second_word.alignments.Add(i, first, type);
      }
    }
  }

  AddSelfAlignments(input);
  AddBoundaryAlignments(input);
}

} // namespace input
