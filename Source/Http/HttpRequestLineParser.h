#pragma once

#include <cstdint>
#include <cstdlib>
#include <string_view>

#include "HttpBase.h"

namespace webserver::http {

struct HttpRequest;

enum class HttpRequestLineParsingState : std::uint8_t;
enum class StepResult : std::uint8_t;

struct RequestLineParsingContext {
  std::size_t chrIdx{};
  int methodEndIndex{};
  int major{};
  int minor{};
  char chr{};
  HttpRequestLineParsingState state{};
};

class HttpRequestLineParser {
 public:
  explicit HttpRequestLineParser(std::string_view requestLine);

  void parse(HttpRequest &outRequest);

 private:
  void _processChar(HttpRequest &outRequest);

  StepResult _parseMethod(HttpRequest &outRequest);
  StepResult _parseSpacesAfterMethod();
  StepResult _parseUri(HttpRequest &outRequest);
  StepResult _parseHttpVersionMajor();
  [[nodiscard]] HttpVersion _getHttpVersion() const;

  // Utils
  static void _expect(char realChar, char expected);
  static void _expectDigit(char chr);
  static bool _isSpaceOrTab(char chr);
  static bool _isAsciiUppercase(char chr);
  static bool _isEndOfLine(const RequestLineParsingContext &parsingContext,
                           std::string_view requestLine);
  static void _updateVersion(int &version, char chr);

  RequestLineParsingContext _context{};
  const std::string_view _requestLine;
};

}  // namespace webserver::http
