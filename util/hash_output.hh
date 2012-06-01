#ifndef UTIL_HASH_OUTPUT__
#define UTIL_HASH_OUTPUT__

#include <boost/functional/hash.hpp>

#include <iterator>

// http://www.sgi.com/tech/stl/OutputIterator.html has rather strict semantics for users of an OutputIterator.  

namespace util {

class HashOutput;

namespace detail {
class HashCombineOnAssign {
  public:
    HashCombineOnAssign() {}

    explicit HashCombineOnAssign(size_t &hashed) : hashed_(&hashed) {}

    template <class T> void operator=(const T &t) {
      boost::hash_combine(*hashed_, t);
    }

  private:
    // Since operator= is rather busy, here's how to actually assign to this.
    friend class ::util::HashOutput;
    void CopyFrom(const HashCombineOnAssign &from) { hashed_ = from.hashed_; }

    size_t *hashed_;
};
} // namespace detail

// OutputIterator that hashes the incoming values together.
class HashOutput {
  public:
    typedef std::output_iterator_tag iterator_category;
    typedef detail::HashCombineOnAssign reference;
    
    // These need to be declared but not used.   
    typedef void value_type;
    typedef void pointer;
    typedef void difference_type;

    HashOutput() {}

    explicit HashOutput(size_t &hashed) : assign_(hashed) { hashed = 0; }

    // Here to prevent HashCombineOnAssign's overactive operator= from being called.
    void operator=(const HashOutput &from) {
      assign_.CopyFrom(from.assign_);
    }

    detail::HashCombineOnAssign &operator*() { return assign_; }

    void operator++(int) const {}

    HashOutput &operator++() { return *this; }

  private:
    detail::HashCombineOnAssign assign_;
};

} // namespace util

#endif // UTIL_HASH_OUTPUT__
