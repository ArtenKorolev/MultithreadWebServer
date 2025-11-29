#include "HttpServer.h"

#include <algorithm>
#include <fstream>
#include <print>
#include <stdexcept>

#include "HttpParser.h"
#include "HttpResponse.h"
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

    _threadPool.enqueue(
        [client = std::move(clientSocket)]() mutable { _serveClient(client); });
  }
}

void HttpServer::_serveClient(const std::unique_ptr<ISocket>& clientSocket) {
  try {
    const auto requestObject{HttpParser::parse(clientSocket->receive())};
    const StaticFileHandler responseBuilder{requestObject};
    const auto response = responseBuilder.handle();
    clientSocket->send(response.toString());
  } catch (const std::exception& e) {
    const auto response = HttpResponse{
        .statusCode = StatusCode::HTTP_400_BAD_REQUEST, .body = e.what()};
    clientSocket->send(response.toString());
  }
}

}  // namespace webserver::net
