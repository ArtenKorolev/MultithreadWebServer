#include "HttpResponse.h"

#include "Utils.h"

namespace webserver::http {

std::string HttpResponse::toString() const {
  const auto statusLine = _generateStatusLine();
  const auto headers = _headersToString();
  return statusLine + "\r\n" + headers + "\r\n" + body + "\r\n\r\n";
}

std::string HttpResponse::_generateStatusLine() const {
  return "HTTP/" + httpVersion + ' ' + _statusCodeToString() + ' ' +
         std::string(generateStatusCodeMap()[statusCode]);
}

std::string HttpResponse::_statusCodeToString() const {
  const auto stausCodeValue = static_cast<int>(statusCode);
  return std::to_string(stausCodeValue);
}

std::string HttpResponse::_headersToString() const {
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
  headersString += std::move(name) + ": " + std::move(value) + "\r\n";
}

}  // namespace webserver::http
