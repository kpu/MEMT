#ifndef _MEMT_Input_Format_h
#define _MEMT_Input_Format_h

#include <ostream>

#include "MEMT/Input/Alignment.hh"

namespace input {

class WordText;
struct Word;
struct Engine;
struct Input;

std::ostream &operator<<(std::ostream &str, const WordText &text);

std::ostream &AlignTypeOut(std::ostream &str, const AlignType &type, char delim = ' ');
std::ostream &operator<<(std::ostream &str, const WordAlignments &align);

std::ostream &operator<<(std::ostream &str, const Word &word);
std::ostream &operator<<(std::ostream &str, const Engine &engine);
std::ostream &operator<<(std::ostream &str, const Input &input);

std::ostream &LaTeXAlignment(std::ostream &str, const std::string &top_title, const Engine &top, const std::string &bottom_title, const Engine &bottom, bool exclude_bounds = true);

}  // namespace input

#endif
