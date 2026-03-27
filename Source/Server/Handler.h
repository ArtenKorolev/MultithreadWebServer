#pragma once

#include <expected>

#include "HttpResponse.h"
#include "Socket.h"

namespace webserver::net {

enum class ConnType : std::uint8_t { CLOSE, KEEP_ALIVE };

using HandlingResult = std::expected<ConnType, http::HttpError>;

class IHandler {
 public:
  [[nodiscard]] virtual HandlingResult handle(ISocket& clientSocket) const = 0;

  virtual ~IHandler() = default;
};

}  // namespace webserver::net
