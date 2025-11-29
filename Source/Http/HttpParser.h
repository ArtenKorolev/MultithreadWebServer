#pragma once

#include <string>

#include "HttpRequest.h"

namespace webserver::http {

class HttpParser {
 public:
  explicit HttpParser(const std::string &request) noexcept : _request{request} {
  }

  [[nodiscard]] HttpRequest parse() const;

 private:
  void _parseRequestLine(HttpRequest &outRequest) const;
  void _parseHeaders(HttpRequest &outRequest) const;
  void _parseBody(HttpRequest &outRequest) const;
  [[nodiscard]] std::string _getHeaders() const;

  static bool _isAsciiUppercase(char chr);

  const std::string &_request;
};

}  // namespace webserver::http
