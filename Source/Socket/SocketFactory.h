#pragma once

#include <unistd.h>

#include <memory>

#include "Socket.h"

namespace webserver::net {

class SocketFactory {
 public:
  [[nodiscard]] static std::unique_ptr<ISocket> newSocket();
};

}  // namespace webserver::net
