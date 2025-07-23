#pragma once

#include <memory>
#include <string>

#include "HostData.h"

namespace webserver::net {

class ISocket {
 public:
  ISocket() = default;
  ISocket(const ISocket &) = delete;
  ISocket(ISocket &&) = delete;
  ISocket &operator=(const ISocket &) = delete;
  ISocket &operator=(ISocket &&) = delete;
  virtual ~ISocket() = default;

  virtual void connect(const HostData &hostData) = 0;
  virtual void bind(std::uint16_t port) = 0;
  virtual std::unique_ptr<ISocket> accept() = 0;
  virtual void listen() = 0;
  virtual void send(const std::string &data) = 0;
  virtual std::string receive() = 0;
};

}  // namespace webserver::net
