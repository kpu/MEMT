/* Hashing for Boost fusion objects.  
 * The hash_value function is placed in boost::fusion so calling just hash_value on it works.  
 */

#include <boost/functional/hash/hash.hpp>
#include <boost/fusion/algorithm/iteration/accumulate.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/utility/enable_if.hpp>

namespace util {
namespace detail {

struct HashCombine {
  template <class T> size_t operator()(size_t value, const T &t) const {
    boost::hash_combine(value, t);
    return value;
  }
  typedef size_t result_type;
};

} // namespace detail
} // namespace util

namespace boost {
namespace fusion {

template <class T> inline typename enable_if<traits::is_sequence<T>, size_t>::type hash_value(const T &t) {
  return boost::fusion::accumulate(t, static_cast<size_t>(0), util::detail::HashCombine());
}

} // namespace fusion
} // namespace boost
