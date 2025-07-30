#pragma once

#include <cstdint>

#include "SocketFactory.h"
#include "ThreadPool.h"

namespace webserver::net {

constexpr auto kDefaultPort = 8000;

class HttpServer {
 public:
  explicit HttpServer(std::uint16_t port = kDefaultPort) : _port{port} {
    _throwIfPortIsInvalid();
    _serverSocket = SocketFactory::newSocket();
    _serverSocket->bind(port);
  }

  HttpServer(const HttpServer &) = delete;
  HttpServer(HttpServer &&) = delete;
  HttpServer &operator=(const HttpServer &) = delete;
  HttpServer &operator=(HttpServer &&) = delete;
  ~HttpServer() = default;

  void startServerLoop();

 private:
  static void _serveClient(std::unique_ptr<ISocket> clientSocket);
  void _throwIfPortIsInvalid() const;

  core::ThreadPool _threadPool;
  std::uint16_t _port;
  std::unique_ptr<ISocket> _serverSocket;
};

}  // namespace webserver::net
