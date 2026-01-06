#pragma once

#include <string>

#include "HttpRequest.h"

namespace webserver::http {

struct HeadersParsingContext;

class HttpParser {
 public:
  explicit HttpParser(std::string request) noexcept
      : _request{std::move(request)} {
  }

  [[nodiscard]] HttpRequest parse() const;

 private:
  [[nodiscard]] std::string_view _getRequestLine() const;

  void _parseHeaders(HttpRequest &outRequest) const;
  void _parseBody(HttpRequest &outRequest) const;
  [[nodiscard]] std::pair<std::size_t, std::size_t> _getHeaders() const;

  static bool _isSpaceOrTab(char chr);
  void _processHeaderChar(HeadersParsingContext &parsingContext,
                          HttpRequest &outRequest) const;

  const std::string _request;
};

}  // namespace webserver::http
