#ifndef UTIL_BOUNDED_I_STREAM__
#define UTIL_BOUNDED_I_STREAM__

#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>

#include <istream>

namespace util {

namespace detail {
class BoundedIStreamDevice : public boost::iostreams::source {
  public:
    BoundedIStreamDevice(std::istream &backend, std::streamsize bound)
      : backend_(backend), bound_(bound) {}

    std::streamsize read(char *out, std::streamsize n) {
      backend_.read(out, std::min(n, bound_));
      if (backend_.eof()) return -1;
      if (backend_.fail()) throw std::ios_base::failure("Backend stream failbit without eof");
      bound_ -= backend_.gcount();
      return backend_.gcount();
    }

    bool Completed() const {
      return bound_ == 0;
    }

  private:
    std::istream &backend_;
    std::streamsize bound_;
};
} // namespace detail

typedef boost::iostreams::stream<detail::BoundedIStreamDevice> BoundedIStream;

} // namespace util

#endif // UTIL_BOUNDED_I_STREAM__
