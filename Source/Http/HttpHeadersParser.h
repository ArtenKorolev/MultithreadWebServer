#pragma once

#include <string_view>

#include "HttpRequest.h"

namespace webserver::http {

struct HeadersParsingContext;

class HttpHeadersParser {
 public:
  explicit HttpHeadersParser(std::string_view headers);

  void parse(HttpRequest &outRequest) const;

 private:
  void _processHeaderChar(HeadersParsingContext &parsingContext,
                          HttpRequest &outRequest) const;

  std::string_view _headers;
};

}  // namespace webserver::http
