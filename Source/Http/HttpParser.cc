#include "HttpParser.h"

#include <fmt/core.h>

#include <string_view>
#include <utility>

#include "HttpRequestLineParser.h"
#include "ParsingUtils.cc"

namespace webserver::http {

constexpr char kCR = '\r';
constexpr std::string_view kCRLF = "\r\n";
constexpr std::string_view kDoubleCRLF = "\r\n\r\n";

#define INLINE __attribute__((always_inline)) inline

HttpRequest HttpParser::parse() const {
  HttpRequest result;

  HttpRequestLineParser{_getRequestLine()}.parse(result);
  _parseHeaders(result);
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

enum class HttpHeadersParsingState : std::uint8_t {
  HEADER_NAME,
  COLON,
  SPACES_AFTER_COLON,
  HEADER_VALUE,
  SPACES_AFTER_HEADER_VALUE,
};

struct HeadersParsingContext {
  HttpHeadersParsingState state{};
  std::string nameBuffer;
  std::string valueBuffer;
  std::size_t chrIdx{};
};

void HttpParser::_parseHeaders(HttpRequest &outRequest) const {
  const auto [headersStart, headersEnd] = _getHeaders();

  HeadersParsingContext parsingContext{
      .state = HttpHeadersParsingState::HEADER_NAME,
      .chrIdx = headersStart,
  };

  for (; std::cmp_less(parsingContext.chrIdx, headersEnd);
       ++parsingContext.chrIdx) {
    _processHeaderChar(parsingContext, outRequest);
  }

  if (!parsingContext.nameBuffer.empty() &&
      !parsingContext.valueBuffer.empty()) {
    outRequest.headers[std::move(parsingContext.nameBuffer)] =
        std::move(parsingContext.valueBuffer);
  }
}

INLINE void HttpParser::_processHeaderChar(
    HeadersParsingContext &parsingContext, HttpRequest &outRequest) const {
  const char chr = _request.at(parsingContext.chrIdx);
  switch (parsingContext.state) {
    case HttpHeadersParsingState::HEADER_NAME:
      if (utils::isSpaceOrTab(chr)) {
        throw std::runtime_error("spaces are not allowed in header name");
      }

      if (chr == ':') {
        parsingContext.state = HttpHeadersParsingState::COLON;
        break;
      }

      parsingContext.nameBuffer +=
          static_cast<char>(std::tolower(static_cast<std::uint8_t>(chr)));
      break;
    case HttpHeadersParsingState::COLON:
      if (utils::isSpaceOrTab(chr)) {
        parsingContext.state = HttpHeadersParsingState::SPACES_AFTER_COLON;
      } else {
        parsingContext.state = HttpHeadersParsingState::HEADER_VALUE;
        goto header_value;
      }
    case HttpHeadersParsingState::SPACES_AFTER_COLON:
      if (utils::isSpaceOrTab(chr)) {
        break;
      }
    case HttpHeadersParsingState::HEADER_VALUE:
    header_value:
      if (utils::isSpaceOrTab(chr)) {
        parsingContext.state =
            HttpHeadersParsingState::SPACES_AFTER_HEADER_VALUE;
        break;
      }
      if (chr == kCR) {
        goto done;
      }

      parsingContext.valueBuffer += chr;
      break;
    case HttpHeadersParsingState::SPACES_AFTER_HEADER_VALUE:
      if (utils::isSpaceOrTab(chr)) {
        break;
      }

    done:
      outRequest.headers[std::move(parsingContext.nameBuffer)] =
          std::move(parsingContext.valueBuffer);
      parsingContext.nameBuffer.clear();
      parsingContext.valueBuffer.clear();
      ++parsingContext.chrIdx;
      parsingContext.state = HttpHeadersParsingState::HEADER_NAME;
  }
}

INLINE std::pair<std::size_t, std::size_t> HttpParser::_getHeaders() const {
  auto start = _request.find(kCRLF);
  if (start == std::string::npos) {
    throw std::runtime_error("Malformed HTTP request: missing CRLF");
  }

  start += kCRLF.size();

  auto end = _request.find(kDoubleCRLF, start);
  if (end == std::string::npos) {
    throw std::runtime_error(R"(Malformed HTTP request: missing \r\n\r\n)");
  }

  return std::make_pair(start, end);
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
