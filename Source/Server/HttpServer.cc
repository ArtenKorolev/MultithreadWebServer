#include "HttpServer.h"

#include <stdexcept>

#include "HttpResponse.h"
#include "SocketFactory.h"
#include "StaticFileHandler.h"

namespace webserver::net {

constexpr auto kMinPort = 1;
constexpr auto kMaxPort = 65535;

using namespace http;

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

    _threadPool.enqueue([client = std::move(clientSocket)]() mutable {
      _serveClient(std::move(client));
    });
  }
}

void HttpServer::_serveClient(std::unique_ptr<ISocket> clientSocket) {
  const auto handleResult = StaticFileHandler::handle(*clientSocket);

  if (!handleResult.has_value()) {
    const auto response = HttpResponse::fromError(handleResult.error());
    clientSocket->send(response.serialize());
  }
}

}  // namespace webserver::net
