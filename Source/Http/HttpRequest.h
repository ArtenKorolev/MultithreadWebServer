#pragma once

#include <string>

#include "HttpBase.h"

namespace webserver::http {

struct HttpRequest {
  MethodType method;
  HTTPVersion httpVersion;
  std::string uri;
  HeadersType headers;
  std::string body;
};

}  // namespace webserver::http
