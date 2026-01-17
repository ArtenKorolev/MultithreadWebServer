#pragma once

#include <string>

#include "HttpRequest.h"

namespace webserver::http {

class HttpParser {
 public:
  explicit HttpParser(std::string request) noexcept
      : _request{std::move(request)} {
  }

  [[nodiscard]] HttpRequest parse() const;

 private:
  [[nodiscard]] std::string_view _getRequestLine() const;
  [[nodiscard]] std::string_view _getHeaders() const;
  void _parseBody(HttpRequest &outRequest) const;

  const std::string _request;
};

}  // namespace webserver::http
