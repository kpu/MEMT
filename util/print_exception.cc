#include <cstdlib>
#include <exception>
#include <iostream>

namespace {

void terminate_handler() {
  try { throw; }
  catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  catch(...) {
    std::cerr << "A non-standard exception was thrown." << std::endl;
  }
  std::abort();
}

struct ForceCall {
  ForceCall() { std::set_terminate(terminate_handler); }
};
const ForceCall kForceCall;

} // namespace
