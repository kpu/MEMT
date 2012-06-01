#include "lm/model.hh"
#include "lm/sa.hh"
#include "lm/sri.hh"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <sys/resource.h>
#include <sys/time.h>

float FloatSec(const struct timeval &tv) {
  return static_cast<float>(tv.tv_sec) + (static_cast<float>(tv.tv_usec) / 1000000000.0);
}

void PrintUsage(const char *message) {
  struct rusage usage;
  if (getrusage(RUSAGE_SELF, &usage)) {
    perror("getrusage");
    return;
  }
  std::cerr << message;
  std::cerr << "user\t" << FloatSec(usage.ru_utime) << "\nsys\t" << FloatSec(usage.ru_stime) << '\n';

  // Linux doesn't set memory usage :-(.  
  std::ifstream status("/proc/self/status", std::ios::in);
  std::string line;
  while (getline(status, line)) {
    if (!strncmp(line.c_str(), "VmRSS:\t", 7)) {
      std::cerr << "rss " << (line.c_str() + 7) << '\n';
      break;
    }
  }
}

template <class Model> void Query(const Model &model) {
  PrintUsage("Loading statistics:\n");
  typename Model::State state, out;
  lm::FullScoreReturn ret;
  std::string word;

  while (std::cin) {
    state = model.BeginSentenceState();
    float total = 0.0;
    bool got = false;
    while (std::cin >> word) {
      got = true;
      lm::WordIndex index = model.GetVocabulary().Index(word);
      ret = model.FullScore(state, index, out);
      total += ret.prob;
      std::cout << word << ' ' << index << ' ' << static_cast<unsigned int>(ret.ngram_length)  << ' ' << ret.prob << ' ';
      state = out;
      if (std::cin.get() == '\n') break;
    }
    if (!got && !std::cin) break;
    ret = model.FullScore(state, model.GetVocabulary().EndSentence(), out);
    total += ret.prob;
    std::cout << "</s> " << model.GetVocabulary().EndSentence() << ' ' << static_cast<unsigned int>(ret.ngram_length)  << ' ' << ret.prob << ' ';
    std::cout << "Total: " << total << '\n';
  }
  PrintUsage("After queries:\n");
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Pass language model then type (sa, sri, or ngram)." << std::endl;
    return 0;
  }
  if (!strcmp(argv[2], "sri")) {
    if (argc < 4) {
      std::cerr << "SRI requires the model order.  Pass it as the 3rd argument." << std::endl;
      return 1;
    }
    char *end;
    int order = strtoul(argv[3], &end, 10);
    if (end == argv[3]) {
      std::cerr << "Pass a numeric order for SRI. " << std::endl;
      return 1;
    }
    lm::sri::Model sri(argv[1], order);
    Query(sri);
  } else if (!strcmp(argv[2], "ngram")) {
    lm::ngram::Model ngram(argv[1]);
    Query(ngram);
  } else if (!strcmp(argv[2], "sa")) {
    std::cerr << "assuming order 8." << std::endl;
    lm::sa::Model sa(argv[1], 8);
    Query(sa);
  } else {
    std::cerr << "type must be sa, sri, or ngram." << std::endl;
  }
  PrintUsage("Total time including destruction:\n");
}
