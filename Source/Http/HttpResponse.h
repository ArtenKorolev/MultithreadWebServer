#pragma once

#include <optional>
#include <string>

#include "HttpBase.h"

namespace webserver::http {

constexpr auto kDefaultHttpVersion = "HTTP/1.1";

struct HttpError {
  StatusCode statusCode;
  std::optional<std::string> message;
};

struct HttpResponse {
  [[nodiscard]] static HttpResponse fromError(const HttpError &error) {
    HttpResponse response;
    response.statusCode = error.statusCode;

    response.headers = {{"Connection", "close"}};

    if (error.message.has_value()) {
      response.body = error.message.value();
      response.headers["Content-Type"] = "text/plain";
    }

    return response;
  }

  [[nodiscard]] std::string serialize() const;

  StatusCode statusCode{StatusCode::HTTP_200_OK};
  std::optional<std::string> body;
  std::string httpVersion{kDefaultHttpVersion};
  HeadersType headers;

 private:
  [[nodiscard]] std::string _generateStatusLine() const;
  [[nodiscard]] std::string _generateHeadersString() const;
  static void _addHeader(std::string &headersString, std::string name,
                         std::string value);
};

}  // namespace webserver::http
