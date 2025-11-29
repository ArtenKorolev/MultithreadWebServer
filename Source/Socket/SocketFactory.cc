#include "SocketFactory.h"

#include <memory>

#include "UnixSocket.h"

namespace webserver::net {

std::unique_ptr<ISocket> SocketFactory::newSocket() {
#if defined(_POSIX_VERSION)
  return std::make_unique<UnixSocket>();
#else
  #error "Unsupported platform"
#endif
}

}  // namespace webserver::net
