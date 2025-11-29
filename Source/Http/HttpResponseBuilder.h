#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

namespace webserver::http {

class HttpResponseBuilder {
 public:
  explicit HttpResponseBuilder(HttpRequest request)
      : _request{std::move(request)} {};

  [[nodiscard]] HttpResponse build() const;

 private:
  [[nodiscard]] static std::string _getContentTypeByFileName(
      const std::string& fileName);

  HttpRequest _request;
};

}  // namespace webserver::http
