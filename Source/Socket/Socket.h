#pragma once

#include <memory>
#include <string>

#include "HostData.h"

namespace webserver::net {

class ISocket {
 public:
  ISocket(const ISocket &) = delete;
  ISocket(ISocket &&) = delete;
  ISocket &operator=(const ISocket &) = delete;
  ISocket &operator=(ISocket &&) = delete;
  virtual ~ISocket() = default;

  virtual void connect(const HostData &hostData) = 0;
  virtual void bind(const HostData &hostData) = 0;
  virtual std::unique_ptr<ISocket> accept() = 0;
  virtual void listen() = 0;
  virtual void send(std::string data) = 0;
  virtual std::string recieve() = 0;
};

}  // namespace webserver::net
