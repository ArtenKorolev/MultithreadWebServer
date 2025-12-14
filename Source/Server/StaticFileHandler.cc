#include "StaticFileHandler.h"

#include <expected>

#include "HttpBase.h"
#include "HttpParser.h"
#include "HttpResponse.h"
#include "Socket.h"

namespace webserver::http {

[[nodiscard]] std::expected<void, HttpError> StaticFileHandler::handle(
    net::ISocket& clientSocket) {
  const auto requestRaw = clientSocket.receive();
  const auto request = HttpParser{requestRaw}.parse();

  const std::filesystem::path fullPath = "public/" + request.uri;

  if (_containsTwoDotsPattern(fullPath)) {
    return std::unexpected<HttpError>{
        {.statusCode = StatusCode::HTTP_400_BAD_REQUEST,
         .message = "Preventing traversal path: '..' found in URI"}};
  }

  if (!std::filesystem::exists(fullPath) ||
      !std::filesystem::is_regular_file(fullPath)) {
    return std::unexpected<HttpError>{
        {.statusCode = StatusCode::HTTP_404_NOT_FOUND}};
  }

  const auto response = HttpResponse{
      .statusCode = StatusCode::HTTP_200_OK,
      .headers = {{"Content-Type", _getMimeTypeByFileName(fullPath)},
                  {"Content-Length",
                   std::to_string(std::filesystem::file_size(fullPath))},
                  {"Connection", "close"}}};

  clientSocket.send(response.serialize());
  clientSocket.sendZeroCopyFile(fullPath);

  return {};
}

inline bool StaticFileHandler::_containsTwoDotsPattern(const std::string& uri) {
  return uri.find("..") != std::string::npos;
}

std::string StaticFileHandler::_getMimeTypeByFileName(
    const std::filesystem::path& fileName) {
  static const std::unordered_map<std::string, std::string>
      extensionToMimeType = {{".html", "text/html"},
                             {".css", "text/css"},
                             {".js", "application/javascript"},
                             {".png", "image/png"},
                             {".jpg", "image/jpeg"},
                             {".jpeg", "image/jpeg"},
                             {".gif", "image/gif"},
                             {".svg", "image/svg+xml"},
                             {".json", "application/json"},
                             {".txt", "text/plain"},
                             {".wasm", "application/wasm"}};

  return extensionToMimeType.at(fileName.extension());
}

}  // namespace webserver::http
