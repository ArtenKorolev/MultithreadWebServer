#pragma once

#include <string>

#include "HttpBase.h"

namespace webserver::http {

constexpr auto kDefaultHttpVersion = "1.1";

struct HttpResponse {
  [[nodiscard]] std::string toString() const;

  StatusCode statusCode;
  std::string body;
  std::string httpVersion{kDefaultHttpVersion};
  HeadersType headers;

 private:
  [[nodiscard]] std::string _generateStatusLine() const;
  [[nodiscard]] std::string _statusCodeToString() const;
  [[nodiscard]] std::string _headersToString() const;
  static void _addHeader(std::string &headersString, std::string name, std::string value);
};

}  // namespace webserver::http
