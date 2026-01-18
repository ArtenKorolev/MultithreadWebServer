#include "HttpParser.h"

#include <fmt/core.h>

#include <string_view>

#include "HttpHeadersParser.h"
#include "HttpRequestLineParser.h"

namespace webserver::http {

constexpr char kCR = '\r';
constexpr std::string_view kCRLF = "\r\n";
constexpr std::string_view kDoubleCRLF = "\r\n\r\n";

#define INLINE __attribute__((always_inline)) inline

HttpRequest HttpParser::parse() const {
  HttpRequest result;

  HttpRequestLineParser{_getRequestLine()}.parse(result);
  HttpHeadersParser{_getHeaders()}.parse(result);
  _parseBody(result);

  return result;
}

INLINE std::string_view HttpParser::_getRequestLine() const {
  const std::size_t endOfRequestLine = _request.find(kCRLF);

  if (endOfRequestLine == std::string::npos) {
    throw std::runtime_error("Expected \\r\\n after request line");
  }

  const std::string_view requestLine{_request.data(), endOfRequestLine};

  if (requestLine.empty()) {
    throw std::runtime_error("empty request line");
  }

  return requestLine;
}

INLINE std::string_view HttpParser::_getHeaders() const {
  auto start = _request.find(kCRLF);

  if (start == std::string::npos) {
    throw std::runtime_error("Malformed HTTP request: missing CRLF");
  }

  start += kCRLF.size();

  auto end = _request.find(kDoubleCRLF, start);

  if (end == std::string::npos) {
    throw std::runtime_error(R"(Malformed HTTP request: missing \r\n\r\n)");
  }

  return {_request.data() + start, end - start};
}

void HttpParser::_parseBody(HttpRequest &outRequest) const {
  const auto bodyStartPosition = _request.find(kDoubleCRLF);

  if (bodyStartPosition == std::string::npos) {
    throw std::runtime_error{R"(Expected \r\n\r\n before body)"};
  }

  const std::size_t bodyPos = bodyStartPosition + kDoubleCRLF.size();
  outRequest.body = _request.substr(bodyPos);
}

}  // namespace webserver::http
