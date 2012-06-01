#include "util/latex_escape.hh"

namespace util {
namespace {

class Replace {
  public:
    Replace() {
      for (unsigned int i = 0; i < 256; ++i) {
        null_separated_bytes_[i * 2] = i;
        null_separated_bytes_[i * 2 + 1] = 0;
        map_[i] = &null_separated_bytes_[i * 2];
      }
      map_[(unsigned char)'<'] = "\\textless ";
      map_[(unsigned char)'>'] = "\\textgreater ";
      map_[(unsigned char)'$'] = "\\$";
      map_[(unsigned char)'_'] = "\\textunderscore ";
      map_[(unsigned char)'{'] = "\\{";
      map_[(unsigned char)'}'] = "\\}";
      map_[(unsigned char)'\\'] = "\\textbacklash ";
      map_[(unsigned char)'%'] = "\\%";
      map_[(unsigned char)'#'] = "\\#";
      map_[(unsigned char)'&'] = "\\&";
    }

    const char *operator[](size_t value) {
      return map_[value];
    }

  private:
    char null_separated_bytes_[512];
    const char *map_[256];
};

Replace replace;

} // namespace

void LatexEscape(const StringPiece &in, std::string &out) {
  out.clear();
  for (const char *i = in.data(); i != in.data() + in.size(); ++i) {
    out.append(replace[*i]);
  }
}

} // namespace util
