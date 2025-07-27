#include "SocketFactory.h"

#include <memory>

#include "UnixSocket.h"

namespace webserver::net {

std::unique_ptr<ISocket> SocketFactory::newSocket() {
#if defined(_POSIX_VERSION)
  return std::make_unique<UnixSocket>();
#else
  throw std::runtime_error("Unsupported platform for socket creation");
#endif
}

}  // namespace webserver::net
