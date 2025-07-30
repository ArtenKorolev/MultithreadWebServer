#include "HttpServer.h"

#include <stdexcept>
#include <thread>

namespace webserver::net {

constexpr auto kMinPort = 1;
constexpr auto kMaxPort = 65535;

void HttpServer::_throwIfPortIsInvalid() const {
  if (_port < kMinPort || _port > kMaxPort) {
    throw std::invalid_argument("Port number must be between 1 and 65535.");
  }
}

void HttpServer::startServerLoop() const {
  _serverSocket->listen();

  while (true) {
    auto clientSocket{_serverSocket->accept()};
    std::thread clientThread([client = std::move(clientSocket)]() mutable {
      HttpServer::_serveClient(std::move(client));
    });
    clientThread.detach();
  }
}

void HttpServer::_serveClient(std::unique_ptr<ISocket> clientSocket) {
  auto request = clientSocket->receive();
  clientSocket->send(
      "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!");
}

}  // namespace webserver::net
