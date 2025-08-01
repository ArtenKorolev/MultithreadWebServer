#include "HttpServer.h"

#include <stdexcept>

namespace webserver::net {

constexpr auto kMinPort = 1;
constexpr auto kMaxPort = 65535;

HttpServer::HttpServer(const std::uint16_t port) : _port{port} {
  _throwIfPortIsInvalid();
  _serverSocket = SocketFactory::newSocket();
  _serverSocket->bind(port);
}

void HttpServer::_throwIfPortIsInvalid() const {
  if (_port < kMinPort || _port > kMaxPort) {
    throw std::invalid_argument("Port number must be between 1 and 65535.");
  }
}

void HttpServer::startServerLoop() {
  _serverSocket->listen();

  while (true) {
    auto clientSocket{_serverSocket->accept()};

    _threadPool.enqueue(
        [client = std::move(clientSocket)]() mutable { _serveClient(client); });
  }
}

void HttpServer::_serveClient(const std::unique_ptr<ISocket> &clientSocket) {
  auto request = clientSocket->receive();  // NOLINT

  clientSocket->send(
      "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!");
}

}  // namespace webserver::net
