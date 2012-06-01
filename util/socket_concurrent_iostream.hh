#ifndef UTIL_SOCKET_CONCURRENT_IOSTREAM__
#define UTIL_SOCKET_CONCURRENT_IOSTREAM__

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>

#include <iosfwd>

/* But Kenneth, boost::asio already has iostream.  Why yes, it does.
 * In fact: http://www.boost.org/users/news/version_1_37_0 says:
 * "Synchronous read, write, accept and connect operations are now thread safe
 * (meaning that it is now permitted to perform concurrent synchronous
 * operations on an individual socket, if supported by the OS)."
 *
 * Take note of "socket."  It's ok to have one thread write to a socket while
 * another reads from it.  However, it's not ok to do this with a single
 * buffered iostream.  Inconveniently, there does not seem to be a built-in
 * way to get two iostreams attached to a single socket.  The packaged
 * streambuf inherits from socket so there can only be one per socket.
 *
 * I have used boost::iostreams to create my own iostreams that reference a
 * single socket.  It's threadsafe to have a ReadSocketStream and a
 * WriteSocketStream active in different threads.  In theory, several of each
 * could be active, but e.g. writer bytes would be interwoven. 
 *
 * Usage: 
 *   ReadSocketDevice reader(socket);
 *   WriteSocketDevice writer(socket);
 *   // Then concurrently:
 *   reader >> foo;
 *   writer << bar;
 */

namespace util {

namespace detail {
class ReadSocketDevice : public boost::iostreams::source {
  public:
    explicit ReadSocketDevice(boost::asio::ip::tcp::socket &sock) : sock_(sock) {}

    std::streamsize read(char *out, std::streamsize n) {
      return sock_.read_some(boost::asio::buffer(out, n)); 
    }

  private:
    boost::asio::ip::tcp::socket &sock_;
};
} // namespace detail

typedef boost::iostreams::stream<detail::ReadSocketDevice> ReadSocketStream;

namespace detail {
class WriteSocketDevice : public boost::iostreams::sink {
  public:
    explicit WriteSocketDevice(boost::asio::ip::tcp::socket &sock) : sock_(sock) {}

    std::streamsize write(const char *in, std::streamsize n) {
      return sock_.write_some(boost::asio::buffer(in, n));
    }

  private:
    boost::asio::ip::tcp::socket &sock_;
};
} // namespace detail

typedef boost::iostreams::stream<detail::WriteSocketDevice> WriteSocketStream;

} // namespace util

#endif // UTIL_SOCKET_CONCURRENT_IOSTREAM__
