#include "HttpServer.h"

#include <stdexcept>

namespace webserver::net {

constexpr auto kMinPort = 1;
constexpr auto kMaxPort = 65535;

void HttpServer::_throwIfPortIsInvalid() const {
  if (_port < kMinPort || _port > kMaxPort) {
    throw std::invalid_argument("Port number must be between 1 and 65535.");
  }
}

}  // namespace webserver::net