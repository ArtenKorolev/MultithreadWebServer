#pragma once

#include <cstdint>

#include "SocketFactory.h"

namespace webserver::net {

constexpr auto kDefaultPort = 8000;

class HttpServer {
 public:
  explicit HttpServer(std::uint16_t port = kDefaultPort) : _port(port) {
    _throwIfPortIsInvalid();
    _serverSocket = SocketFactory::newSocket();
    _serverSocket->bind(port);
  }

 private:
  void _throwIfPortIsInvalid() const;

  std::uint16_t _port;
  std::unique_ptr<ISocket> _serverSocket;
};

}  // namespace webserver::net
