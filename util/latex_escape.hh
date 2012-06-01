#ifndef UTIL_LATEX_ESCAPE__
#define UTIL_LATEX_ESCAPE__

#include "util/string_piece.hh"

#include <string>

namespace util {

// Escape characters for LaTeX.  This isn't a formally formal escape, but does list what I encounter.
void LatexEscape(const StringPiece &in, std::string &out);

} // namespace util
#endif // UTIL_LATEX_ESCAPE__
