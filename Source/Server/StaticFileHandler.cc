#include "StaticFileHandler.h"

#include <expected>

#include "HttpBase.h"
#include "HttpParser.h"
#include "HttpResponse.h"
#include "Socket.h"

namespace webserver::http {

StaticFileHandler::StaticFileHandler(const std::string& contentDirectory)
    : _contentDirectory{contentDirectory} {
}

[[nodiscard]] std::expected<void, HttpError> StaticFileHandler::handle(
    net::ISocket& clientSocket) const {
  const auto requestRaw{clientSocket.receive()};
  const auto request{HttpParser{requestRaw}.parse()};

  const auto fullPath{_getFullPath(request.uri)};

  const auto validationResult = _validateUri(fullPath);

  if (!validationResult.has_value()) {
    return validationResult;
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

std::expected<void, HttpError> StaticFileHandler::_validateUri(
    const std::filesystem::path& path) {
  if (_containsTwoDotsPattern(path)) {
    return std::unexpected<HttpError>{
        {.statusCode = StatusCode::HTTP_400_BAD_REQUEST,
         .message = "Preventing traversal path: '..' found in URI"}};
  }

  if (!std::filesystem::exists(path) ||
      !std::filesystem::is_regular_file(path)) {
    return std::unexpected<HttpError>{
        {.statusCode = StatusCode::HTTP_404_NOT_FOUND}};
  }

  return {};
}

std::filesystem::path StaticFileHandler::_getFullPath(
    const std::string& uri) const {
  const auto qsMarkPos = uri.find('?');

  // TODO: replace logic of parsing uri into query and params to HttpParser
  const auto preparedUri =
      qsMarkPos == std::string::npos ? uri : uri.substr(0, qsMarkPos);

  return _contentDirectory + preparedUri;
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
                             {".php", "application/x-php"},
                             {".txt", "text/plain"},
                             {".wasm", "application/wasm"}};

  return extensionToMimeType.at(fileName.extension());
}

}  // namespace webserver::http
