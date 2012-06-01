#include "MEMT/Input/Format.hh"

#include "MEMT/Input/Input.hh"
#include "MEMT/Input/Word.hh"

#include "util/latex_escape.hh"

#include <set>

using namespace std;

namespace input {

ostream &operator<<(ostream &str, const WordText &text) {
  return str << "original=\"" << text.Original() << "\", canonical=\"" << text.Canonical() << "\", punctuation=" << text.IsPunctuation();
}

ostream &AlignTypeOut(ostream &str, const AlignType &type, char delim) {
  bool do_delim = false;
  for (unsigned int i = 0; i < AL_COUNT; ++i) {
    if (type & (1 << i)) {
      if (do_delim) {
        str << ' ';
      } else {
        do_delim = true;
      }
      str << kAlignTypeNames[i];
    }
  }
  return str;
}

ostream &operator<<(ostream &str, const WordAlignments &align) {
  for (unsigned int other = 0; other < align.size(); ++other) {
    const AlignmentInfo &info = align.Ask(other);
    if (info.IsNone()) continue;
    str << "Alignment engine " << other << " offset " << info.offset << ", type ";
    AlignTypeOut(str, info.type) << '\n';
  }
  return str;
}

ostream &operator<<(ostream &str, const Word &word) {
  return str << word.text << '\n' << word.alignments;
}

ostream &operator<<(ostream &str, const Engine &engine) {
  for (unsigned int i = 0; i < engine.words.size(); ++i) {
    str << "Word " << i << ": " << engine.words[i] << '\n';
  }
  return str;
}

ostream &operator<<(ostream &str, const Input &input) {
  for (unsigned int i = 0; i < input.engines.size(); ++i) {
    str << "Engine " << i << ":\n";   
    const Engine &engine = input.engines[i];
    for (unsigned int e = 0; e < engine.words.size(); ++e) {
      const Word &word = engine.words[e];
      str << "Word " << e << ": " << word.text << '\n';
      for (unsigned int other = 0; other < word.alignments.size(); ++other) {
        const AlignmentInfo &info = word.alignments.Ask(other);
        if (info.IsNone()) continue;

        str << "Alignment engine " << other << " offset " << info.offset << " text \"" << input.GetWord(other, info.offset).text.Canonical() << "\" type ";
        AlignTypeOut(str, info.type) << '\n';
      }
    }
    str << '\n';
  }
  return str;
}

namespace {

std::ostream &LaTeXLine(std::ostream &str, const std::string &prefix, const Engine &engine, bool exclude_bounds) {
  std::string escaped;
  for (std::vector<Word>::const_iterator i(engine.words.begin() + exclude_bounds); i != engine.words.end() - exclude_bounds; ++i) {
    util::LatexEscape(i->text.Original(), escaped);
    str << " \\al{" << prefix << (i - engine.words.begin()) << "}{" << escaped << "} ";
  }
  return str;
}

std::ostream &LaTeXJustAlignments(std::ostream &str, const Engine &top, const Engine &bottom, bool exclude_bounds) {
  for (unsigned int i = exclude_bounds; i < top.words.size() - exclude_bounds; ++i) {
    const AlignmentInfo &al = top.words[i].alignments.Ask(bottom.number);
    if (!al.type) continue;
    str << "\\draw[";
    AlignTypeOut(str, al.type, ',');
    str << "] (t" << i << ".south) -- (b" << al.offset << ".north);\n";
  }
  return str;
}

} // namespace

std::ostream &LaTeXAlignment(std::ostream &str, const std::string &top_title, const Engine &top, const std::string &bottom_title, const Engine &bottom, bool exclude_bounds) {
  str 
    << "\\begin{tikzpicture}[textbase,node distance=\\aldist]\n"
    << "\\begin{scope}[start chain]\n"
    << "\\node[on chain](t){\\textbf{" << top_title << "}};\n";
  LaTeXLine(str, "t", top, exclude_bounds) << '\n';
  str
    << "\\end{scope}\n"
    << "\\begin{scope}[start chain]\n"
    << "\\node[on chain, anchor=north east](b) at ($(t.south east) + (0pt, -\\aligheight)$){\\textbf{" << bottom_title << "}};\n";
  LaTeXLine(str, "b", bottom, exclude_bounds);
  str
    << "\\end{scope}\n";
  LaTeXJustAlignments(str, top, bottom, exclude_bounds);
  str
    << "\\end{tikzpicture}\n";
  return str;
}

} // namespace input
