#pragma once

#include <string>

#include "HttpBase.h"

namespace webserver::http {

struct HttpRequest {
  HttpMethod method;
  HttpVersion httpVersion;
  std::string uri;
  HeadersType headers;
  std::string body;
};

}  // namespace webserver::http
