#include "StaticFileHandler.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace webserver::http {

HttpResponse StaticFileHandler::handle() const {
  HeadersType headers = {{"Server", "webserver"}, {"Connection", "close"}};

  if (_containsTwoDotsPattern(_request.uri)) {
    return HttpResponse{
        .statusCode = StatusCode::HTTP_400_BAD_REQUEST,
        .body = "Unsafe pattern found in uri: '..'",
        .headers = headers,
    };
  }

  const std::filesystem::path fullFilePath = "public/" + _request.uri;

  const auto isPathCorrect = std::filesystem::exists(fullFilePath) &&
                             std::filesystem::is_regular_file(fullFilePath);

  if (!isPathCorrect) {
    return HttpResponse{.statusCode = StatusCode::HTTP_404_NOT_FOUND,
                        .body = "Requested file not found",
                        .headers = headers};
  }

  const std::ifstream fileStream{fullFilePath};

  if (!fileStream.is_open()) {
    return HttpResponse{
        .statusCode = StatusCode::HTTP_500_INTERNAL_SERVER_ERROR,
        .body = "Cannot open file",
        .headers = headers};
  }

  std::stringstream stringStream;
  stringStream << fileStream.rdbuf();

  return HttpResponse{.statusCode = StatusCode::HTTP_200_OK,
                      .body = stringStream.str(),
                      .headers = headers};
}

bool StaticFileHandler::_containsTwoDotsPattern(const std::string& uri) {
  return uri.find("..") != std::string::npos;
}

std::string StaticFileHandler::_getMimeTypeByFileName(
    const std::string& fileName) {
  return "text/html";  // TODO complete logic
}

}  // namespace webserver::http
