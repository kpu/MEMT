#ifndef _MEMT_Strategy_Graph_Coverage_Hypothesis_h
#define _MEMT_Strategy_Graph_Coverage_Hypothesis_h

#include "util/hash_output.hh"

#include <boost/dynamic_bitset.hpp>

namespace strategy {
namespace graph {
namespace coverage {

class Hypothesis {
  public:
    Hypothesis() {}
  private:
    friend class Sentence;
    friend size_t hash_value(const Hypothesis &hyp);
    friend bool operator==(const Hypothesis &left, const Hypothesis &right);

    boost::dynamic_bitset<unsigned int> bits_;
};

inline size_t hash_value(const Hypothesis &hyp) {
  size_t ret = 0;
  to_block_range(hyp.bits_, util::HashOutput(ret));
  return ret;
}

inline bool operator==(const Hypothesis &left, const Hypothesis &right) {
  return left.bits_ == right.bits_;
}

} // namespace coverage
} // namespace graph
} // namespace strategy
#endif // _MEMT_Strategy_Graph_Coverage_Hypothesis_h
