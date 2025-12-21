#pragma once

#include <expected>
#include <filesystem>

#include "HttpBase.h"
#include "HttpResponse.h"
#include "Socket.h"

namespace webserver::http {

class StaticFileHandler {
 public:
  [[nodiscard]] static std::expected<void, HttpError> handle(
      net::ISocket &clientSocket);

 private:
  static std::filesystem::path _getFullPath(const std::string &uri);
  [[nodiscard]] static bool _containsTwoDotsPattern(const std::string &uri);
  [[nodiscard]] static std::string _getMimeTypeByFileName(
      const std::filesystem::path &fileName);
};

}  // namespace webserver::http
