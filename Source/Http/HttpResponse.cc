#include "HttpResponse.h"

#include <fmt/core.h>

#include "Utils.h"

namespace webserver::http {

std::string HttpResponse::toString() const {
  return fmt::format("{}\r\n{}\r\n{}", _generateStatusLine(),
                     _generateHeadersString(), body);
}

std::string HttpResponse::_generateStatusLine() const {
  return fmt::format("{} {} {}", httpVersion,
                     static_cast<std::uint16_t>(statusCode),
                     generateStatusCodeMap()[statusCode]);
}

std::string HttpResponse::_generateHeadersString() const {
  std::string headersString;

  _addHeader(headersString, "Content-Length", std::to_string(body.size()));
  _addHeader(headersString, "Date", utils::getCurrentDate());

  for (const auto& [header, value] : headers) {
    _addHeader(headersString, header, value);
  }

  return headersString;
}

void HttpResponse::_addHeader(std::string& headersString, std::string name,
                              std::string value) {
  headersString += fmt::format("{}: {}\r\n", std::move(name), std::move(value));
}

}  // namespace webserver::http
