#ifndef UTIL_BARRIER__
#define UTIL_BARRIER__

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

namespace util {

class Barrier {
  public:
    explicit Barrier(size_t n) : n_(n) {
      assert(n);
    }

    bool Decrement() {
      // There are faster ways to do this hidden in boost/detail/sp_counted_base_*, but they're poorly factored for such.
      boost::unique_lock<boost::mutex> lock(mutex_);
      return (0 == --n_);
    }

  private:
    size_t n_;
    boost::mutex mutex_;
};

} // namespace util

#endif // UTIL_BARRIER__
