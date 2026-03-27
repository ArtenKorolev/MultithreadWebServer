#include "StaticFileHandler.h"

#include "Handler.h"
#include "HttpBase.h"
#include "HttpParser.h"
#include "IniParser.h"
#include "Socket.h"

namespace webserver::http {

StaticFileHandler::StaticFileHandler(std::string contentDirectory)
    : _contentDirectory{std::move(contentDirectory)} {
}

[[nodiscard]] net::HandlingResult StaticFileHandler::handle(
    net::ISocket& clientSocket) const {
  const auto requestRaw{clientSocket.receive()};
  const auto request{HttpParser{requestRaw}.parse()};

  net::ConnType connType = net::ConnType::CLOSE;

  if (request.headers.contains("Connection")) {
    connType = request.headers.at("Connection") == "close"
                   ? net::ConnType::CLOSE
                   : net::ConnType::KEEP_ALIVE;
  } else if (request.httpVersion >= HttpVersion::HTTP_1_1) {
    connType = net::ConnType::KEEP_ALIVE;  // In HTTP versions >= 1.1
                                           // Conneciton: keep-alive is implied
  }

  const auto fullPath{_getFullPath(request.uri)};

  const auto validationResult = _validateUri(fullPath);

  if (!validationResult.has_value()) {
    return std::unexpected<HttpError>(validationResult.error());
  }

  const auto response = HttpResponse{
      .statusCode = StatusCode::HTTP_200_OK,
      .headers = {{"Content-Type", _getMimeTypeByFileName(fullPath)},
                  {"Content-Length",
                   std::to_string(std::filesystem::file_size(fullPath))},
                  {"Connection",
                   connType == net::ConnType::CLOSE ? "close" : "keep-alive"}}};

  clientSocket.send(response.serialize());
  clientSocket.sendZeroCopyFile(fullPath);

  return connType;
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
  static const core::UmapStrStr extensionToMimeType = {
      {".html", "text/html"},
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
