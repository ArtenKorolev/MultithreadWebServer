#include "HttpParser.h"

#include <fmt/core.h>

#include <unordered_set>
#include <utility>

namespace webserver::http {

constexpr char kCR = '\r';
constexpr std::string_view kCRLF = "\r\n";
constexpr std::string_view kDoubleCRLF = "\r\n\r\n";

HttpRequest HttpParser::parse() {
  HttpRequest result;

  _parseRequestLine(result);
  _parseHeaders(result);
  _parseBody(result);

  return result;
}

static std::unordered_set<char> getSymbolsAllowedInURI() {
  static std::unordered_set<char> symbolsAllowedInURI = {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
      'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b',
      'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
      'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3',
      '4', '5', '6', '7', '8', '9', '-', '.', '_', '~', '!', '$', '&', '\'',
      '(', ')', '*', '+', ',', ';', '=', ':', '@', '/', '?'};

  return symbolsAllowedInURI;
}

template <typename State>
struct ParsingContext {
  int chrIdx;
  int methodEndIndex;
  int major;
  int minor;
  char chr;
  State state;
};

enum class HttpRequestLineParsingState : std::uint8_t {
  METHOD,
  SPACES_AFTER_METHOD,
  URI,
  SPACES_AFTER_URI,
  HTTP_VERSION_H,
  HTTP_VERSION_HT,
  HTTP_VERSION_HTT,
  HTTP_VERSION_HTTP,
  HTTP_VERSION_SLASH,
  HTTP_VERSION_MAJOR,
  HTTP_VERSION_DOT,
  HTTP_VERSION_MINOR,
  END_OF_HTTP_VERSION,
  SPACES_AFTER_VERSION,
};

enum class StepResult : std::uint8_t { CONTINUE, BREAK };

void HttpParser::_parseRequestLine(HttpRequest &outRequest) const {
  const std::size_t endOfRequestLine = _request.find(kCRLF);

  if (endOfRequestLine == std::string::npos) {
    throw std::runtime_error("Expected \\r\\n after request line");
  }

  const std::string_view requestLine{_request.data(), endOfRequestLine};

  if (requestLine.empty()) {
    throw std::runtime_error("empty request line");
  }

  ParsingContext<HttpRequestLineParsingState> parsingContext{
      .chrIdx = 0,
      .methodEndIndex = 0,
      .major = 0,
      .minor = 0,
      .state = HttpRequestLineParsingState::METHOD,
  };

  for (; parsingContext.chrIdx < requestLine.size(); ++parsingContext.chrIdx) {
    parsingContext.chr = requestLine[parsingContext.chrIdx];
    switch (parsingContext.state) {
      case HttpRequestLineParsingState::METHOD:
        if (_parseMethod(parsingContext, requestLine, outRequest) ==
            StepResult::BREAK) {
          break;
        }
      case HttpRequestLineParsingState::SPACES_AFTER_METHOD:
        if (_parseSpacesAfterMethod(parsingContext) == StepResult::BREAK) {
          break;
        }
      case HttpRequestLineParsingState::URI:
        if (_parseUri(parsingContext, outRequest) == StepResult::BREAK) {
          break;
        }
      case HttpRequestLineParsingState::SPACES_AFTER_URI:
        if (_isSpaceOrTab(parsingContext.chr)) {
          break;
        }
      case HttpRequestLineParsingState::HTTP_VERSION_H:
        _expect(parsingContext.chr, 'H');
        parsingContext.state = HttpRequestLineParsingState::HTTP_VERSION_HT;
        break;
      case HttpRequestLineParsingState::HTTP_VERSION_HT:
        _expect(parsingContext.chr, 'T');
        parsingContext.state = HttpRequestLineParsingState::HTTP_VERSION_HTT;
        break;
      case HttpRequestLineParsingState::HTTP_VERSION_HTT:
        _expect(parsingContext.chr, 'T');
        parsingContext.state = HttpRequestLineParsingState::HTTP_VERSION_HTTP;
        break;
      case HttpRequestLineParsingState::HTTP_VERSION_HTTP:
        _expect(parsingContext.chr, 'P');
        parsingContext.state = HttpRequestLineParsingState::HTTP_VERSION_SLASH;
        break;
      case HttpRequestLineParsingState::HTTP_VERSION_SLASH:
        _expect(parsingContext.chr, '/');
        parsingContext.state = HttpRequestLineParsingState::HTTP_VERSION_MAJOR;
        break;
      case HttpRequestLineParsingState::HTTP_VERSION_MAJOR:
        _parseHttpVersionMajor(parsingContext, requestLine);
        break;
      case HttpRequestLineParsingState::HTTP_VERSION_DOT:
        _expectDigit(parsingContext.chr);
      case HttpRequestLineParsingState::HTTP_VERSION_MINOR:
        parsingContext.minor =
            parsingContext.minor * 10 + parsingContext.chr - '0';
        parsingContext.state = HttpRequestLineParsingState::END_OF_HTTP_VERSION;
        break;
      case HttpRequestLineParsingState::END_OF_HTTP_VERSION:
        if (_isEndOfLine(parsingContext, requestLine)) {
          break;
        }
        parsingContext.state =
            HttpRequestLineParsingState::SPACES_AFTER_VERSION;
        break;
      case HttpRequestLineParsingState::SPACES_AFTER_VERSION:
        if (_isSpaceOrTab(parsingContext.chr)) {
          break;
        }

        throw std::runtime_error("invalid characters after version");
    }
  }

  outRequest.httpVersion = _getHttpVersion(parsingContext);
}

inline StepResult HttpParser::_parseMethod(
    ParsingContext<HttpRequestLineParsingState> &parsingContext,
    const std::string_view requestLine, HttpRequest &outRequest) {
  if (_isEndOfLine(parsingContext, requestLine)) {
    throw std::runtime_error("missing uri and version");
  }

  if (_isSpaceOrTab(parsingContext.chr)) {
    outRequest.method = getStrToMethodMap().at(
        requestLine.substr(0, parsingContext.methodEndIndex));
    parsingContext.state = HttpRequestLineParsingState::SPACES_AFTER_METHOD;
    return StepResult::BREAK;
  }

  if (!_isAsciiUppercase(parsingContext.chr)) {
    throw std::runtime_error(
        fmt::format("invalid character in method: '{}'", parsingContext.chr));
  }

  ++parsingContext.methodEndIndex;

  return StepResult::BREAK;
}

inline StepResult HttpParser::_parseSpacesAfterMethod(
    ParsingContext<HttpRequestLineParsingState> &parsingContext) {
  if (_isSpaceOrTab(parsingContext.chr)) {
    return StepResult::BREAK;
  }

  parsingContext.state = HttpRequestLineParsingState::URI;
  return StepResult::CONTINUE;
}

inline StepResult HttpParser::_parseUri(
    ParsingContext<HttpRequestLineParsingState> &parsingContext,
    HttpRequest &outRequest) {
  if (_isSpaceOrTab(parsingContext.chr)) {
    parsingContext.state = HttpRequestLineParsingState::SPACES_AFTER_URI;
    return StepResult::BREAK;
  }

  if (!getSymbolsAllowedInURI().contains(parsingContext.chr)) {
    throw std::runtime_error("invalid character in uri");
  }

  outRequest.uri += parsingContext.chr;

  return StepResult::BREAK;
}

inline StepResult HttpParser::_parseHttpVersionMajor(
    ParsingContext<HttpRequestLineParsingState> &parsingContext,
    const std::string_view requestLine) {
  if (parsingContext.chr == '.') {
    if (_isEndOfLine(parsingContext, requestLine)) {
      throw std::runtime_error("Dot cannot be last");
    }
    parsingContext.state = HttpRequestLineParsingState::HTTP_VERSION_DOT;
    return StepResult::BREAK;
  }

  _expectDigit(parsingContext.chr);

  parsingContext.major = parsingContext.major * 10 + parsingContext.chr - '0';
  return StepResult::BREAK;
}

inline HttpVersion HttpParser::_getHttpVersion(
    const ParsingContext<HttpRequestLineParsingState> &parsingContext) {
  if (parsingContext.major == 0 && parsingContext.minor == 9) {
    return HttpVersion::HTTP_0_9;
  }
  if (parsingContext.major == 1 && parsingContext.minor == 0) {
    return HttpVersion::HTTP_1_0;
  }
  if (parsingContext.major == 1 && parsingContext.minor == 1) {
    return HttpVersion::HTTP_1_1;
  }
  if (parsingContext.major == 2 && parsingContext.minor == 0) {
    return HttpVersion::HTTP_2;
  }
  if (parsingContext.major == 3 && parsingContext.minor == 0) {
    return HttpVersion::HTTP_3;
  }

  throw std::runtime_error("invalid HTTP version");
}

inline void HttpParser::_expect(const char realChar, const char expected) {
  if (realChar != expected) {
    throw std::runtime_error(
        fmt::format("expected '{}' but got '{}'", expected, realChar));
  }
}

inline void HttpParser::_expectDigit(char chr) {
  if (chr < '0' || chr > '9') {
    throw std::runtime_error(fmt::format("expected digit but got '{}'", chr));
  }
}

inline bool HttpParser::_isSpaceOrTab(const char chr) {
  return chr == ' ' || chr == '\t';
}

inline bool HttpParser::_isAsciiUppercase(const char chr) {
  return chr >= 'A' && chr <= 'Z';
}

inline bool HttpParser::_isEndOfLine(
    const ParsingContext<HttpRequestLineParsingState> &parsingContext,
    const std::string_view requestLine) {
  return parsingContext.chrIdx == requestLine.size() - 1;
}

enum class HttpHeadersParsingState : std::uint8_t {
  HEADER_NAME,
  COLON,
  SPACES_AFTER_COLON,
  HEADER_VALUE,
  SPACES_AFTER_HEADER_VALUE,
};

// This code also is going to be refactored
// TODO refactor
void HttpParser::_parseHeaders(HttpRequest &outRequest) {
  const auto [headersStart, headersEnd] = _getHeaders();

  auto parsingState = HttpHeadersParsingState::HEADER_NAME;

  std::string nameBuffer;
  std::string valueBuffer;

  int i = headersStart;
  for (; std::cmp_less(i, headersEnd); ++i) {
    const char chr = _request[i];
    switch (parsingState) {
      case HttpHeadersParsingState::HEADER_NAME:
        if (_isSpaceOrTab(chr)) {
          throw std::runtime_error("spaces are not allowed in header name");
        }

        if (chr == ':') {
          parsingState = HttpHeadersParsingState::COLON;
          break;
        }

        nameBuffer += std::tolower(static_cast<std::uint8_t>(chr));
        break;
      case HttpHeadersParsingState::COLON:
        if (_isSpaceOrTab(chr)) {
          parsingState = HttpHeadersParsingState::SPACES_AFTER_COLON;
        } else {
          parsingState = HttpHeadersParsingState::HEADER_VALUE;
          goto header_value;
        }
      case HttpHeadersParsingState::SPACES_AFTER_COLON:
        if (_isSpaceOrTab(chr)) {
          break;
        }
      case HttpHeadersParsingState::HEADER_VALUE:
      header_value:
        if (_isSpaceOrTab(chr)) {
          parsingState = HttpHeadersParsingState::SPACES_AFTER_HEADER_VALUE;
          break;
        }
        if (chr == kCR) {
          goto done;
        }

        valueBuffer += chr;
        break;
      case HttpHeadersParsingState::SPACES_AFTER_HEADER_VALUE:
        if (_isSpaceOrTab(chr)) {
          break;
        }

      done:
        outRequest.headers[std::move(nameBuffer)] = std::move(valueBuffer);
        ++i;
        parsingState = HttpHeadersParsingState::HEADER_NAME;
    }
  }

  if (!nameBuffer.empty() && !valueBuffer.empty()) {
    outRequest.headers[std::move(nameBuffer)] = std::move(valueBuffer);
  }
}

inline std::pair<std::size_t, std::size_t> HttpParser::_getHeaders() const {
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
