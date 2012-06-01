#include "MEMT/Input/AlignType.hh"
#include "MEMT/Input/Config.hh"
#include "MEMT/Input/Input.hh"
#include "MEMT/Input/ReadFromJava.hh"

#include <boost/unordered_map.hpp>

#include <functional>
#include <iostream>
#include <string>
#include <vector>

size_t AlignTypeToIndex(const input::AlignType t) {
  if (t & input::AL_EXACT) {
    return 0;
  } else if (t & input::AL_SNOWBALL_STEM) {
    return 1;
  } else if (t & input::AL_WN_SYNONYMY) {
    return 2;
  } else if (t & input::AL_PARAPHRASE) {
    return 3;
  } else {
    return 4;
  }
}

const char *kAlignTypeIndexNames[] = {"exact", "snowball_stem", "wn_synonymy", "paraphrase", "other"};

typedef boost::unordered_map<std::pair<std::string, std::string>, size_t> Counter;

struct GreaterBySize : public std::binary_function<const Counter::value_type *, const Counter::value_type*, bool> {
  bool operator()(const Counter::value_type *left, const Counter::value_type *right) const {
    return left->second > right->second;
  }
};

std::ostream &PrintWithSpace(std::ostream &to, const std::string &str, size_t width) {
  to << str;
  if (str.size() > width) return to;
  for (size_t i = 0; i < width - str.size(); ++i) {
    to << ' ';
  }
  return to;
}

void DumpTable(const Counter &table) {
  size_t total = 0;
  std::vector<const Counter::value_type*> sorted;
  for (boost::unordered_map<std::pair<std::string, std::string>, size_t>::const_iterator i = table.begin(); i != table.end(); ++i) {
    sorted.push_back(&*i);
  }
  std::sort(sorted.begin(), sorted.end(), GreaterBySize());
  for (std::vector<const Counter::value_type*>::const_iterator i(sorted.begin()); i != sorted.end(); ++i) {
    PrintWithSpace(std::cout, (*i)->first.first, 20);
    PrintWithSpace(std::cout, (*i)->first.second, 20);
    std::cout << (*i)->second << '\n';
    total += (*i)->second;
  }
  std::cout << total << '\n';
}

int main() {
  size_t unaligned = 0;
  std::vector<Counter> counts(5);
  input::Input input;
  input::Config config;
  config.lowercase_before_lm = false;
  config.transitive = false;

  for (unsigned int v = 0; ; ++v) {
    try {
      input::ReadFromJava(config, std::cin, input);
    }
    catch (std::ios_base::failure &f) {
      if (std::cin.eof()) break;
      throw;
    }
    for (unsigned int e = 0; e < input.engines.size(); ++e) {
      for (unsigned int w = 0; w < input.engines[e].words.size(); ++w) {
        const input::WordAlignments &align = input.GetWord(e, w).alignments;
        for (unsigned int o = e + 1; o < input.engines.size(); ++o) {
          const input::AlignmentInfo &info = align.Ask(o);
          if (!info.type) {
            ++unaligned;
            continue;
          }
          std::pair<std::string, std::string> match(
              input.GetWord(e, w).text.Canonical(),
              input.GetWord(o, info.offset).text.Canonical());
          if (match.first > match.second) swap(match.first, match.second);
          ++counts[AlignTypeToIndex(info.type)][match];
        }
      }
    }
  }
  for (std::vector<Counter>::const_iterator i(counts.begin()); i != counts.end(); ++i) {
    std::cout << kAlignTypeIndexNames[i - counts.begin()] << '\n';
    DumpTable(*i);
    std::cout << "\n\n";
  }
  std::cout << "Unaligned engine pairs: " << unaligned << std::endl;
}
