#include "HttpServer.h"

#include <stdexcept>

#include "HttpResponse.h"
#include "SocketFactory.h"
#include "StaticFileHandler.h"

namespace webserver::net {

constexpr auto kMinPort{1};
constexpr auto kMaxPort{65535};

using namespace http;

HttpServer::HttpServer(config::Config config)
    : _config{std::move(config)}, _threadPool{_config.threadsCount} {
  _throwIfPortIsInvalid();
  _serverSocket = SocketFactory::newSocket();
  _serverSocket->bind(_config.port);
}

void HttpServer::_throwIfPortIsInvalid() const {
  if (_config.port < kMinPort || _config.port > kMaxPort) {
    throw std::invalid_argument("Port number must be between 1 and 65535.");
  }
}

void HttpServer::startServerLoop() {
  shutdownRequested.store(false);
  _serverSocket->listen();

  while (!shutdownRequested.load()) {
    try {
      auto clientSocket{_serverSocket->accept()};

      if (shutdownRequested.load()) {
        break;
      }

      _threadPool.enqueue([client = std::move(clientSocket), this]() mutable {
        _serveClient(std::move(client));
      });
    } catch (const std::exception &e) {
      if (shutdownRequested.load()) {
        break;
      }

      throw;
    }
  }

  _serverSocket->close();
  _threadPool.stop();
}

void HttpServer::_serveClient(std::unique_ptr<ISocket> clientSocket) const {
  const StaticFileHandler handler{_config.contentDirectory};
  const auto handleResult = handler.handle(*clientSocket);

  if (!handleResult.has_value()) {
    const auto response{HttpResponse::fromError(handleResult.error())};
    clientSocket->send(response.serialize());
  }
}

}  // namespace webserver::net
