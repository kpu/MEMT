#include "util/hash_fusion.hh"

#define BOOST_TEST_MODULE HashFusionTest
#include <boost/fusion/container/vector.hpp>
#include <boost/test/unit_test.hpp>

namespace {

BOOST_AUTO_TEST_CASE(Empty) {
  boost::fusion::vector<> vec;
  BOOST_CHECK_EQUAL(static_cast<size_t>(0), hash_value(vec));
}

BOOST_AUTO_TEST_CASE(Single) {
  boost::fusion::vector<int> vec(1);
  size_t hash_accum = 0;
  boost::hash_combine(hash_accum, (int)1);
  BOOST_CHECK_EQUAL(hash_accum, hash_value(vec));
}

} // namespace
