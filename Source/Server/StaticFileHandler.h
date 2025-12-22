#pragma once

#include <expected>
#include <filesystem>

#include "HttpBase.h"
#include "HttpResponse.h"
#include "Socket.h"

namespace webserver::http {

class StaticFileHandler {
 public:
  explicit StaticFileHandler(const std::string &contentDirectory);

  [[nodiscard]] std::expected<void, HttpError> handle(
      net::ISocket &clientSocket) const;

 private:
  std::filesystem::path _getFullPath(const std::string &uri) const;
  [[nodiscard]] static bool _containsTwoDotsPattern(const std::string &uri);
  [[nodiscard]] static std::string _getMimeTypeByFileName(
      const std::filesystem::path &fileName);

  const std::string &_contentDirectory;
};

}  // namespace webserver::http
