#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

namespace webserver::http {

class StaticFileHandler {
 public:
  explicit StaticFileHandler(HttpRequest request)
      : _request{std::move(request)} {};

  [[nodiscard]] HttpResponse handle() const;

 private:
  [[nodiscard]] static bool _containsTwoDotsPattern(const std::string& uri);
  [[nodiscard]] static std::string _getMimeTypeByFileName(
      const std::string& fileName);

  HttpRequest _request;
};

}  // namespace webserver::http
