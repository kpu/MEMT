#ifndef _MEMT_Controller_CoordWrite_h
#define _MEMT_Controller_CoordWrite_h

#include <boost/thread/mutex.hpp>

#include <ostream>

// Coordinate writes so output is returned in blocks that the reader understands.
namespace controller {

class CoordStream {
  public:
    explicit CoordStream(std::ostream &stream) : stream_(stream) {}

  private:
    friend class CoordWrite;

    std::ostream &stream_;

    boost::mutex mutex_;
};

class CoordWrite {
  public:
    explicit CoordWrite(CoordStream &coord)
      : stream_(coord.stream_), lock_(coord.mutex_) {
    }

    std::ostream &Get() {  return stream_;  }

    std::ostream &operator*() { return stream_; }
    std::ostream *operator->() { return &stream_; }

    operator std::ostream &() {
      return stream_;
    }

  private:
    std::ostream &stream_;
    boost::unique_lock<boost::mutex> lock_;
};

} // namespace controller

#endif // _MEMT_Controller_CoordWrite_h
