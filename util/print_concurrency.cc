#include <boost/thread/thread.hpp>

#include <iostream>

int main() {
  std::cout << boost::thread::hardware_concurrency() << std::endl;
}
