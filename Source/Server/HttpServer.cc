#include "HttpServer.h"

#include <print>
#include <stdexcept>

#include "Config.h"
#include "Handler.h"
#include "HttpResponse.h"
#include "SocketFactory.h"

namespace webserver::net {

constexpr auto kMinPort{1};
constexpr auto kMaxPort{65535};

using namespace http;

HttpServer::HttpServer(config::Config config, const IHandler& handler)
    : _config{std::move(config)},
      _threadPool{_config.threadsCount},
      _handler{handler} {
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

  std::println("Listening on localhost:{}", _config.port);
  std::println("Content directory: {}/", _config.contentDirectory);

  while (!shutdownRequested.load()) {
    try {
      auto clientSocket{_serverSocket->accept()};

      if (shutdownRequested.load()) {
        break;
      }

      _threadPool.enqueue([client = std::move(clientSocket), this]() mutable {
        _serveClient(std::move(client));
      });
    } catch (const std::exception& e) {
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
  while (true) {
    const auto handleResult = _handler.handle(*clientSocket);

    if (!handleResult.has_value()) {
      std::println("Handling error, message: {}, status code: {}",
                   handleResult.error().message.value_or("null"),
                   static_cast<int>(handleResult.error().statusCode));

      const auto response{HttpResponse::fromError(handleResult.error())};
      clientSocket->send(response.serialize());
      break;
    }

    switch (handleResult.value()) {
      case ConnType::CLOSE:
        return;  // close connection
      default:
        break;
    }
  }
}

}  // namespace webserver::net
