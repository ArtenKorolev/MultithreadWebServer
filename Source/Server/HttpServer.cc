#include "HttpServer.h"

#include <algorithm>
#include <fstream>
#include <print>
#include <stdexcept>

#include "HttpParser.h"
#include "HttpResponse.h"
#include "HttpResponseBuilder.h"

namespace webserver::net {

constexpr auto kMinPort = 1;
constexpr auto kMaxPort = 65535;

using namespace http;

HttpServer::HttpServer(const std::uint16_t port) : _port{port} {
  _throwIfPortIsInvalid();
  _serverSocket = SocketFactory::newSocket();
  _serverSocket->bind(port);
}

void HttpServer::_throwIfPortIsInvalid() const {
  if (_port < kMinPort || _port > kMaxPort) {
    throw std::invalid_argument("Port number must be between 1 and 65535.");
  }
}

void HttpServer::startServerLoop() {
  _serverSocket->listen();

  while (true) {
    auto clientSocket{_serverSocket->accept()};

    _threadPool.enqueue(
        [client = std::move(clientSocket)]() mutable { _serveClient(client); });
  }
}

void HttpServer::_serveClient(const std::unique_ptr<ISocket>& clientSocket) {
  try {
    const auto requestObject{HttpParser::parse(clientSocket->receive())};
    const HttpResponseBuilder responseBuilder{requestObject};
    const auto response = responseBuilder.build();
    clientSocket->send(response.toString());
  } catch (const std::exception& e) {
    const auto response = HttpResponse{
        .statusCode = StatusCode::HTTP_400_BAD_REQUEST, .body = e.what()};
    clientSocket->send(response.toString());
  }
}

//
// HttpResponse HttpServer::_buildResponseForRequest(const HttpRequest& request) {
//   HttpResponse response;
//
//   if (std::ranges::starts_with(request.httpVersion, "HTTP/1")) {
//     response = {
//         .statusCode = StatusCode::HTTP_505_HTTP_VERSION_NOT_SUPPORTED,
//         .body = "Server doesn't support HTTP version: " + request.httpVersion,
//         .headers = {{"Connection", "close"},
//                     {"Server", "webserver"},
//                     {"Content-Type", "text/plain"}}};
//   } else if (request.uri.size() == 1 && request.uri[0] == '/') {
//     std::ifstream file{"public/welcome.html"};
//     std::stringstream buffer;
//     buffer << file.rdbuf();
//
//     response = {.statusCode = StatusCode::HTTP_200_OK,
//                 .body = buffer.str(),
//                 .headers = {{"Connection", "close"},
//                             {"Server", "webserver"},
//                             {"Content-Type", "text/html"}}};
//   }
//
//   else if (std::ifstream stream{"public/" + request.uri}; !stream.is_open()) {
//     response = {.statusCode = StatusCode::HTTP_404_NOT_FOUND,
//                 .body = "Can't find file " + request.uri,
//                 .headers = {{"Connection", "close"},
//                             {"Server", "webserver"},
//                             {"Content-Type", "text/plain"}}};
//   } else {
//     std::stringstream stringStream;
//     stringStream << stream.rdbuf();
//
//     response = {.statusCode = StatusCode::HTTP_200_OK,
//                 .body = stringStream.str(),
//                 .headers = {{"Connection", "close"},
//                             {"Server", "webserver"},
//                             {"Content-Type", request.uri == "/favicon.ico"
//                                                  ? "image/png"
//                                                  : "text/html"}}};
//   }
//
//   return response;
// }

}  // namespace webserver::net
