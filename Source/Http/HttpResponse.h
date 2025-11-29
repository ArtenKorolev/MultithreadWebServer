#pragma once

#include <string>

#include "HttpBase.h"

namespace webserver::http {

constexpr auto kDefaultHttpVersion = "HTTP/1.1";

struct HttpResponse {
  [[nodiscard]] std::string toString() const;

  StatusCode statusCode;
  std::string body;
  std::string httpVersion{kDefaultHttpVersion};
  HeadersType headers;

 private:
  [[nodiscard]] std::string _generateStatusLine() const;
  [[nodiscard]] std::string _generateHeadersString() const;
  static void _addHeader(std::string &headersString, std::string name,
                         std::string value);
};

}  // namespace webserver::http
