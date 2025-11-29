#include "HttpParser.h"

#include <format>
#include <unordered_set>

#include "Utils.h"

namespace webserver::http {

constexpr char kCR = '\r';
constexpr char kLF = '\n';
constexpr std::string_view kCRLF = "\r\n";
constexpr std::string_view kDoubleCRLF = "\r\n\r\n";

HttpRequest HttpParser::parse(const std::string &request) {
  HttpRequest result;

  _parseRequestLine(request, result);
  _parseHeaders(request, result);
  _parseBody(request, result);

  return result;
}

enum class HTTP_REQUEST_LINE_PARSING_STATE : std::uint8_t {
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

// This code is complete garbage, but it's going to be refactored soon
// TODO refactor
void HttpParser::_parseRequestLine(const std::string &request,
                                   HttpRequest &outRequest) {
  const std::size_t endOfRequestLine = request.find(kCRLF);

  if (endOfRequestLine == std::string::npos) {
    throw std::runtime_error("Expected \\r\\n after request line");
  }

  const std::string_view requestLine{request.data(), endOfRequestLine};

  auto parsingState = HTTP_REQUEST_LINE_PARSING_STATE::METHOD;
  int methodEndPointer = 0;
  int major = 0;
  int minor = 0;

  int i = 0;
  for (i; i < requestLine.size(); ++i) {
    const auto chr = requestLine[i];
    switch (parsingState) {
      case HTTP_REQUEST_LINE_PARSING_STATE::METHOD:
        if (chr == kCR || chr == kLF || i == requestLine.size() - 1) {
          throw std::runtime_error("missing uri and version");
        }

        if (chr == ' ') {
          const std::string_view method =
              requestLine.substr(0, methodEndPointer);
          outRequest.method = generateMethodsMap().at(method);
          parsingState = HTTP_REQUEST_LINE_PARSING_STATE::SPACES_AFTER_METHOD;
          break;
        }

        if (!_isAsciiUppercase(chr)) {
          throw std::runtime_error(
              std::format("invalid character in method: '{}'", chr));
        }

        methodEndPointer += 1;
        break;
      case HTTP_REQUEST_LINE_PARSING_STATE::SPACES_AFTER_METHOD:
        if (chr == ' ') {
          break;
        }

        parsingState = HTTP_REQUEST_LINE_PARSING_STATE::URI;
        // fallthrough
      case HTTP_REQUEST_LINE_PARSING_STATE::URI:
        if (chr == ' ') {
          parsingState = HTTP_REQUEST_LINE_PARSING_STATE::SPACES_AFTER_URI;
          break;
        }

        if (!getSymbolsAllowedInURI().contains(chr)) {
          throw std::runtime_error("invalid character in uri");
        }

        outRequest.uri += chr;

        break;
      case HTTP_REQUEST_LINE_PARSING_STATE::SPACES_AFTER_URI:
        if (chr == ' ') {
          break;
        }

        // fallthrough
      case HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_H:
        if (chr != 'H') {
          throw std::runtime_error("invalid character in version, expected: H");
        }

        parsingState = HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_HT;
        break;
      case HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_HT:
        if (chr != 'T') {
          throw std::runtime_error("invalid character in version, expected: T");
        }

        parsingState = HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_HTT;
        break;
      case HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_HTT:
        if (chr != 'T') {
          throw std::runtime_error("invalid character in version, expected: T");
        }

        parsingState = HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_HTTP;
        break;
      case HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_HTTP:
        if (chr != 'P') {
          throw std::runtime_error("invalid character in version, expected: P");
        }

        parsingState = HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_SLASH;
        break;
      case HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_SLASH:
        if (chr != '/') {
          throw std::runtime_error("invalid character in version, expected: /");
        }

        parsingState = HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_MAJOR;
        break;
      case HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_MAJOR:
        if (chr == '.') {
          if (i == requestLine.size() - 1) {
            throw std::runtime_error("Dot cannot be last");
          }
          parsingState = HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_DOT;
          break;
        }

        if (chr < '0' || chr > '9') {
          throw std::runtime_error("Version major number should be a digit");
        }

        major = major * 10 + chr - '0';
        break;
      case HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_DOT:

        if (chr < '0' || chr > '9') {
          throw std::runtime_error("Version minor number should be a digit");
        }
      case HTTP_REQUEST_LINE_PARSING_STATE::HTTP_VERSION_MINOR:
        minor = minor * 10 + chr - '0';
        parsingState = HTTP_REQUEST_LINE_PARSING_STATE::END_OF_HTTP_VERSION;
        break;
      case HTTP_REQUEST_LINE_PARSING_STATE::END_OF_HTTP_VERSION:
        if (i == requestLine.size() - 1) {
          break;
        }
        parsingState = HTTP_REQUEST_LINE_PARSING_STATE::SPACES_AFTER_VERSION;
        break;
      case HTTP_REQUEST_LINE_PARSING_STATE::SPACES_AFTER_VERSION:
        if (chr == ' ') {
          break;
        }

        throw std::runtime_error("invalid characters after version");
    }
  }

  if (major == 0 && minor == 9) {
    outRequest.httpVersion = HTTPVersion::HTTP_0_9;
  } else if (major == 1 && minor == 0) {
    outRequest.httpVersion = HTTPVersion::HTTP_1_0;
  } else if (major == 1 && minor == 1) {
    outRequest.httpVersion = HTTPVersion::HTTP_1_1;
  } else if (major == 2 && minor == 0) {
    outRequest.httpVersion = HTTPVersion::HTTP_2;
  } else if (major == 3 && minor == 0) {
    outRequest.httpVersion = HTTPVersion::HTTP_3;
  } else {
    throw std::runtime_error("invalid HTTP version");
  }
}

bool HttpParser::_isAsciiUppercase(const char chr) {
  return chr >= 'A' && chr <= 'Z';
}

enum class HTTP_HEADERS_PARSING_STATE : std::uint8_t {
  HEADER_NAME,
  COLON,
  SPACES_AFTER_COLON,
  HEADER_VALUE,
  SPACES_AFTER_HEADER_VALUE,
};

// This code also is going to be refactored
// TODO refactor
void HttpParser::_parseHeaders(const std::string &request,
                               HttpRequest &outRequest) {
  const auto headers = _extractHeaders(request);

  auto parsingState = HTTP_HEADERS_PARSING_STATE::HEADER_NAME;

  std::string nameBuffer;
  std::string valueBuffer;

  int i;
  for (i = 0; i < headers.size(); ++i) {
    const char chr = headers[i];
    switch (parsingState) {
      case HTTP_HEADERS_PARSING_STATE::HEADER_NAME:
        if (chr == ' ' || chr == '\t') {
          throw std::runtime_error("spaces are not allowed in header name");
        }

        if (chr == ':') {
          parsingState = HTTP_HEADERS_PARSING_STATE::COLON;
          break;
        }

        nameBuffer += std::tolower(static_cast<std::uint8_t>(chr));
        break;
      case HTTP_HEADERS_PARSING_STATE::COLON:
        if (chr == ' ' || chr == '\t') {
          parsingState = HTTP_HEADERS_PARSING_STATE::SPACES_AFTER_COLON;
        } else {
          parsingState = HTTP_HEADERS_PARSING_STATE::HEADER_VALUE;
          goto header_value;
        }
      case HTTP_HEADERS_PARSING_STATE::SPACES_AFTER_COLON:
        if (chr == ' ' || chr == '\t') {
          break;
        }
      case HTTP_HEADERS_PARSING_STATE::HEADER_VALUE:
      header_value:
        if (chr == ' ' || chr == '\t') {
          parsingState = HTTP_HEADERS_PARSING_STATE::SPACES_AFTER_HEADER_VALUE;
          break;
        }
        if (chr == kCR) {
          goto done;
        }

        valueBuffer += chr;
        break;
      case HTTP_HEADERS_PARSING_STATE::SPACES_AFTER_HEADER_VALUE:
        if (chr == ' ' || chr == '\t') {
          break;
        }

      done:
        outRequest.headers[std::move(nameBuffer)] = std::move(valueBuffer);
        ++i;
        parsingState = HTTP_HEADERS_PARSING_STATE::HEADER_NAME;
    }
  }

  if (!nameBuffer.empty() && !valueBuffer.empty()) {
    outRequest.headers[std::move(nameBuffer)] = std::move(valueBuffer);
  }
}

std::string HttpParser::_extractHeaders(const std::string &request) {
  auto start = request.find(kCRLF);
  if (start == std::string::npos) {
    throw std::runtime_error("Malformed HTTP request: missing CRLF");
  }

  start += kCRLF.size();

  auto end = request.find(kDoubleCRLF, start);
  if (end == std::string::npos) {
    throw std::runtime_error(R"(Malformed HTTP request: missing \r\n\r\n)");
  }

  return request.substr(start, end - start);
}

void HttpParser::_parseBody(const std::string &request,
                            HttpRequest &outRequest) {
  const auto bodyStartPosition = request.find(kDoubleCRLF);

  if (bodyStartPosition == std::string::npos) {
    throw std::runtime_error{R"(Expected \r\n\r\n before body)"};
  }

  const std::size_t bodyPos = bodyStartPosition + kDoubleCRLF.size();
  outRequest.body = request.substr(bodyPos);
}

}  // namespace webserver::http
