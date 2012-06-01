#include "util/hash_output.hh"

#define BOOST_TEST_MODULE HashOutputTest
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <vector>

namespace util {
namespace {

BOOST_AUTO_TEST_CASE(vector_copy) {
  std::vector<int> vec;
  vec.push_back(1);
  vec.push_back(14008);
  vec.push_back(783712947);

  size_t hash;
  HashOutput hasher(hash);
  std::copy(vec.begin(), vec.end(), hasher);

  size_t compare = 0;
  for (std::vector<int>::const_iterator i = vec.begin(); i != vec.end(); ++i) {
    boost::hash_combine(compare, *i);
  }

  BOOST_CHECK_EQUAL(compare, hash);
}

} // namespace
} // namespace util
