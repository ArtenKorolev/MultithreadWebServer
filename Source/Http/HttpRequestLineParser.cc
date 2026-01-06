#include "HttpRequestLineParser.h"

#include <fmt/core.h>

#include <array>

#include "HttpRequest.h"

namespace webserver::http {

#define INLINE __attribute__((always_inline)) inline

constexpr auto kUriTableSize = 256;

static constexpr void fillUriTableWithAlphaCharacters(
    std::array<bool, kUriTableSize> &uriTable) {
  for (int chrCode = 'A'; chrCode <= 'Z'; ++chrCode) {
    uriTable.at(chrCode) = true;
  }

  for (int chrCode = 'a'; chrCode <= 'z'; ++chrCode) {
    uriTable.at(chrCode) = true;
  }
}

static constexpr void fillUriTableWithNumericCharacters(
    std::array<bool, kUriTableSize> &uriTable) {
  for (int chrCode = '0'; chrCode <= '9'; ++chrCode) {
    uriTable.at(chrCode) = true;
  }
}

static constexpr void fillUriTableWithSpecialCharacters(
    std::array<bool, kUriTableSize> &uriTable) {
  constexpr std::array<char, 19> specials = {'-',  '.', '_', '~', '!', '$', '&',
                                             '\'', '(', ')', '*', '+', ',', ';',
                                             '=',  ':', '@', '/', '?'};

  for (char specialChr : specials) {
    uriTable.at(specialChr) = true;
  }
}

static constexpr std::array<bool, kUriTableSize> makeUriSymbolsTable() {
  std::array<bool, kUriTableSize> uriTable{};

  fillUriTableWithAlphaCharacters(uriTable);
  fillUriTableWithNumericCharacters(uriTable);
  fillUriTableWithSpecialCharacters(uriTable);

  return uriTable;
}

static constexpr std::array<bool, kUriTableSize> uriSymbolsTable =
    makeUriSymbolsTable();

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
  HTTP_VERSION_MAJOR_START,
  HTTP_VERSION_MAJOR,
  HTTP_VERSION_DOT,
  HTTP_VERSION_MINOR,
  END_OF_HTTP_VERSION,
  SPACES_AFTER_VERSION,
};

HttpRequestLineParser::HttpRequestLineParser(const std::string_view requestLine)
    : _requestLine(requestLine) {
}

void HttpRequestLineParser::parse(HttpRequest &outRequest) {
  _context = {
      .state = HttpRequestLineParsingState::METHOD,
  };

  for (; _context.chrIdx < _requestLine.size(); ++_context.chrIdx) {
    _context.chr = _requestLine.at(_context.chrIdx);
    _processChar(outRequest);
  }

  outRequest.httpVersion = _getHttpVersion();
}

enum class StepResult : std::uint8_t { CONTINUE, BREAK };

INLINE void HttpRequestLineParser::_processChar(HttpRequest &outRequest) {
  switch (_context.state) {
    case HttpRequestLineParsingState::METHOD:
      if (_parseMethod(outRequest) == StepResult::BREAK) {
        break;
      }
    case HttpRequestLineParsingState::SPACES_AFTER_METHOD:
      if (_parseSpacesAfterMethod() == StepResult::BREAK) {
        break;
      }
    case HttpRequestLineParsingState::URI:
      if (_parseUri(outRequest) == StepResult::BREAK) {
        break;
      }
    case HttpRequestLineParsingState::SPACES_AFTER_URI:
      if (_isSpaceOrTab(_context.chr)) {
        break;
      }
    case HttpRequestLineParsingState::HTTP_VERSION_H:
      _expect(_context.chr, 'H');
      _context.state = HttpRequestLineParsingState::HTTP_VERSION_HT;
      break;
    case HttpRequestLineParsingState::HTTP_VERSION_HT:
      _expect(_context.chr, 'T');
      _context.state = HttpRequestLineParsingState::HTTP_VERSION_HTT;
      break;
    case HttpRequestLineParsingState::HTTP_VERSION_HTT:
      _expect(_context.chr, 'T');
      _context.state = HttpRequestLineParsingState::HTTP_VERSION_HTTP;
      break;
    case HttpRequestLineParsingState::HTTP_VERSION_HTTP:
      _expect(_context.chr, 'P');
      _context.state = HttpRequestLineParsingState::HTTP_VERSION_SLASH;
      break;
    case HttpRequestLineParsingState::HTTP_VERSION_SLASH:
      _expect(_context.chr, '/');
      _context.state = HttpRequestLineParsingState::HTTP_VERSION_MAJOR_START;
      break;
    case HttpRequestLineParsingState::HTTP_VERSION_MAJOR_START:
      _expectDigit(_context.chr);
      _updateVersion(_context.major, _context.chr);
      _context.state = HttpRequestLineParsingState::HTTP_VERSION_MAJOR;
      break;
    case HttpRequestLineParsingState::HTTP_VERSION_MAJOR:
      _parseHttpVersionMajor();
      break;
    case HttpRequestLineParsingState::HTTP_VERSION_DOT:
      _expectDigit(_context.chr);
    case HttpRequestLineParsingState::HTTP_VERSION_MINOR:
      _updateVersion(_context.minor, _context.chr);
      _context.state = HttpRequestLineParsingState::END_OF_HTTP_VERSION;
      break;
    case HttpRequestLineParsingState::END_OF_HTTP_VERSION:
      if (_isEndOfLine(_context, _requestLine)) {
        break;
      }
      _context.state = HttpRequestLineParsingState::SPACES_AFTER_VERSION;
      break;
    case HttpRequestLineParsingState::SPACES_AFTER_VERSION:
      if (_isSpaceOrTab(_context.chr)) {
        break;
      }

      throw std::runtime_error("invalid characters after version");
  }
}

INLINE StepResult HttpRequestLineParser::_parseMethod(HttpRequest &outRequest) {
  if (_isEndOfLine(_context, _requestLine)) {
    throw std::runtime_error("missing uri and version");
  }

  if (_isSpaceOrTab(_context.chr)) {
    outRequest.method =
        getStrToMethodMap().at(_requestLine.substr(0, _context.methodEndIndex));
    _context.state = HttpRequestLineParsingState::SPACES_AFTER_METHOD;
    return StepResult::BREAK;
  }

  if (!_isAsciiUppercase(_context.chr)) {
    throw std::runtime_error(
        fmt::format("invalid character in method: '{}'", _context.chr));
  }

  ++_context.methodEndIndex;

  return StepResult::BREAK;
}

INLINE StepResult HttpRequestLineParser::_parseSpacesAfterMethod() {
  if (_isSpaceOrTab(_context.chr)) {
    return StepResult::BREAK;
  }

  _context.state = HttpRequestLineParsingState::URI;
  return StepResult::CONTINUE;
}

INLINE StepResult HttpRequestLineParser::_parseUri(HttpRequest &outRequest) {
  if (_isSpaceOrTab(_context.chr)) {
    _context.state = HttpRequestLineParsingState::SPACES_AFTER_URI;
    return StepResult::BREAK;
  }

  if (!uriSymbolsTable.at(_context.chr)) {
    throw std::runtime_error("invalid character in uri");
  }

  outRequest.uri += _context.chr;

  return StepResult::BREAK;
}

INLINE StepResult HttpRequestLineParser::_parseHttpVersionMajor() {
  if (_context.chr == '.') {
    if (_isEndOfLine(_context, _requestLine)) {
      throw std::runtime_error("Dot cannot be last");
    }
    _context.state = HttpRequestLineParsingState::HTTP_VERSION_DOT;
    return StepResult::BREAK;
  }

  _expectDigit(_context.chr);

  _updateVersion(_context.major, _context.chr);
  return StepResult::BREAK;
}

INLINE void HttpRequestLineParser::_updateVersion(int &version,
                                                  const char chr) {
  version = (version * 10) + (chr - '0');  // NOLINT
}

INLINE HttpVersion HttpRequestLineParser::_getHttpVersion() const {
  if (_context.major == 0 && _context.minor == 9) {  // NOLINT
    return HttpVersion::HTTP_0_9;
  }
  if (_context.major == 1 && _context.minor == 0) {
    return HttpVersion::HTTP_1_0;
  }
  if (_context.major == 1 && _context.minor == 1) {
    return HttpVersion::HTTP_1_1;
  }
  if (_context.major == 2 && _context.minor == 0) {
    return HttpVersion::HTTP_2;
  }
  if (_context.major == 3 && _context.minor == 0) {
    return HttpVersion::HTTP_3;
  }

  throw std::runtime_error("invalid HTTP version");
}

INLINE void HttpRequestLineParser::_expect(const char realChar,
                                           const char expected) {
  if (realChar != expected) {
    throw std::runtime_error(
        fmt::format("expected '{}' but got '{}'", expected, realChar));
  }
}

INLINE void HttpRequestLineParser::_expectDigit(char chr) {
  if (chr < '0' || chr > '9') {
    throw std::runtime_error(fmt::format("expected digit but got '{}'", chr));
  }
}

INLINE bool HttpRequestLineParser::_isSpaceOrTab(const char chr) {
  return chr == ' ' || chr == '\t';
}

INLINE bool HttpRequestLineParser::_isAsciiUppercase(const char chr) {
  return chr >= 'A' && chr <= 'Z';
}

INLINE bool HttpRequestLineParser::_isEndOfLine(
    const RequestLineParsingContext &parsingContext,
    const std::string_view requestLine) {
  return parsingContext.chrIdx == requestLine.size() - 1;
}

}  // namespace webserver::http
