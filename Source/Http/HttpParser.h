#pragma once

#include <string>

#include "HttpRequest.h"

namespace webserver::http {

// TODO: Replace request string to class field
class HttpParser {
 public:
  [[nodiscard]] static HttpRequest parse(const std::string &request);

 private:
  static void _parseRequestLine(const std::string &request,
                                HttpRequest &outRequest);
  static bool _isAsciiUppercase(char chr);
  static void _parseHeaders(const std::string &request,
                            HttpRequest &outRequest);
  static void _parseBody(const std::string &request, HttpRequest &outRequest);
  static std::string _extractHeaders(const std::string &request);
};

}  // namespace webserver::http
