#pragma once

#include <string>

#include "HttpRequest.h"

namespace webserver::http {

enum class HttpRequestLineParsingState : std::uint8_t;

template <typename State>
struct ParsingContext;

enum class StepResult : std::uint8_t;

class HttpParser {
 public:
  explicit HttpParser(std::string request) noexcept
      : _request{std::move(request)} {
  }

  [[nodiscard]] HttpRequest parse();

 private:
  void _parseRequestLine(HttpRequest &outRequest) const;
  void _parseHeaders(HttpRequest &outRequest);
  void _parseBody(HttpRequest &outRequest) const;
  [[nodiscard]] std::pair<std::size_t, std::size_t> _getHeaders() const;

  static StepResult _parseMethod(
      ParsingContext<HttpRequestLineParsingState> &parsingContext,
      std::string_view requestLine, HttpRequest &outRequest);
  static StepResult _parseSpacesAfterMethod(
      ParsingContext<HttpRequestLineParsingState> &parsingContext);
  static StepResult _parseUri(
      ParsingContext<HttpRequestLineParsingState> &parsingContext,
      HttpRequest &outRequest);
  static StepResult _parseHttpVersionMajor(
      ParsingContext<HttpRequestLineParsingState> &parsingContext,
      std::string_view requestLine);
  static HttpVersion _getHttpVersion(
      const ParsingContext<HttpRequestLineParsingState> &parsingContext);
  static void _expect(char realChar, char expected);
  static void _expectDigit(char chr);
  static bool _isSpaceOrTab(char chr);
  static bool _isAsciiUppercase(char chr);
  static bool _isEndOfLine(
      const ParsingContext<HttpRequestLineParsingState> &parsingContext,
      std::string_view requestLine);

  const std::string _request;
};

}  // namespace webserver::http
