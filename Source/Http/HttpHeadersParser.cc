#include <HttpHeadersParser.h>

#include <utility>

#include "ParsingUtils.h"

namespace webserver::http {

#define INLINE __attribute__((always_inline)) inline

HttpHeadersParser::HttpHeadersParser(const std::string_view headers)
    : _headers{headers} {
}

constexpr char kCR = '\r';

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

void HttpHeadersParser::parse(HttpRequest &outRequest) const {
  HeadersParsingContext parsingContext{
      .state = HttpHeadersParsingState::HEADER_NAME,
      .chrIdx = 0,
  };

  for (; std::cmp_less(parsingContext.chrIdx, _headers.length());
       ++parsingContext.chrIdx) {
    _processHeaderChar(parsingContext, outRequest);
  }

  if (!parsingContext.nameBuffer.empty() &&
      !parsingContext.valueBuffer.empty()) {
    outRequest.headers[std::move(parsingContext.nameBuffer)] =
        std::move(parsingContext.valueBuffer);
  }
}

INLINE void HttpHeadersParser::_processHeaderChar(
    HeadersParsingContext &parsingContext, HttpRequest &outRequest) const {
  const char chr = _headers.at(parsingContext.chrIdx);
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

}  // namespace webserver::http
