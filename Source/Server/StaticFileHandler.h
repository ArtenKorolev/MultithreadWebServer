#pragma once

#include <expected>
#include <filesystem>

#include "Handler.h"
#include "HttpResponse.h"
#include "Socket.h"

namespace webserver::http {

class StaticFileHandler : public net::IHandler {
 public:
  explicit StaticFileHandler(std::string contentDirectory);

  [[nodiscard]] net::HandlingResult handle(
      net::ISocket &clientSocket) const override;

 private:
  [[nodiscard]] static std::expected<void, HttpError> _validateUri(
      const std::filesystem::path &path);
  [[nodiscard]] std::filesystem::path _getFullPath(
      const std::string &uri) const;
  [[nodiscard]] static bool _containsTwoDotsPattern(const std::string &uri);
  [[nodiscard]] static std::string _getMimeTypeByFileName(
      const std::filesystem::path &fileName);

  std::string _contentDirectory;
};

}  // namespace webserver::http
