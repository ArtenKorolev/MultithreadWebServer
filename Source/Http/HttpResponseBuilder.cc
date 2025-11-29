#include "HttpResponseBuilder.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace webserver::http {

HttpResponse HttpResponseBuilder::build() const {
  static const HeadersType kDefaultHeaders = {{"Server", "webserver"},
                                              {"Connection", "close"}};

  std::cout << _request.uri << '\n';

  for (int i = 0; i < _request.uri.size(); ++i) {
    if (i == _request.uri.size() - 1) {
      break;
    }
    if (_request.uri[i] == '.' && _request.uri[i + 1] == '.') {
      return HttpResponse{
          .statusCode = StatusCode::HTTP_400_BAD_REQUEST,
          .body = "Unsafe pattern found in uri: '..'",
          .headers = kDefaultHeaders,
      };
    }
  }

  const auto fullFilePath = "public/" + _request.uri;

  const auto isPathCorrect = std::filesystem::exists(fullFilePath) &&
                             std::filesystem::is_regular_file(fullFilePath);

  if (!isPathCorrect) {
    return HttpResponse{.statusCode = StatusCode::HTTP_404_NOT_FOUND,
                        .body = "Requested file not found",
                        .headers = kDefaultHeaders};
  }

  const std::ifstream fileStream{fullFilePath};

  if (!fileStream.is_open()) {
    return HttpResponse{
        .statusCode = StatusCode::HTTP_500_INTERNAL_SERVER_ERROR,
        .body = "Cannot open file",
        .headers = kDefaultHeaders};
  }

  std::stringstream stringStream;
  stringStream << fileStream.rdbuf();

  return HttpResponse{.statusCode = StatusCode::HTTP_200_OK,
                      .body = stringStream.str(),
                      .headers = kDefaultHeaders};
}
std::string HttpResponseBuilder::_getContentTypeByFileName(
    const std::string& fileName) {
  return "text/html";
}

}  // namespace webserver::http
