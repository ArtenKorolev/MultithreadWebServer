#pragma once

#include <cstdint>

#include "Config.h"
#include "Socket.h"
#include "ThreadPool.h"

inline std::atomic<bool> shutdownRequested{false};

namespace webserver::net {

constexpr auto kDefaultPort = 8000;

class HttpServer {
 public:
  explicit HttpServer(config::Config config);

  HttpServer(const HttpServer &) = delete;
  HttpServer(HttpServer &&) = delete;
  HttpServer &operator=(const HttpServer &) = delete;
  HttpServer &operator=(HttpServer &&) = delete;
  ~HttpServer() = default;

  void startServerLoop();

 private:
  void _serveClient(std::unique_ptr<ISocket> clientSocket) const;
  void _throwIfPortIsInvalid() const;

  const config::Config _config;
  core::ThreadPool _threadPool;
  std::unique_ptr<ISocket> _serverSocket;
};

}  // namespace webserver::net
