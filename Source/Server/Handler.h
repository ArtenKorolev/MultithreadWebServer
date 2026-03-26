#pragma once

#include <expected>

#include "HttpResponse.h"
#include "Socket.h"

namespace webserver::net {

using HandlingResult = std::expected<void, http::HttpError>;

class IHandler {
 public:
  [[nodiscard]] virtual HandlingResult handle(ISocket &clientSocket) const = 0;

  virtual ~IHandler() = default;
};

}  // namespace webserver::net
