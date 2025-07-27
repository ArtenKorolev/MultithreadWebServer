#include "UnixSocket.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <memory>
#include <stdexcept>

#include "HostData.h"

constexpr auto BUFFER_SIZE = 1024;

namespace webserver::net {

UnixSocket::UnixSocket() : _socketFd{::socket(AF_INET, SOCK_STREAM, 0)} {
  if (!_isValidSocket()) {
    throw std::runtime_error("Unable to initialize socket");
  }
}

UnixSocket::UnixSocket(int fileDescriptor) : _socketFd{fileDescriptor} {
  if (!_isValidSocket()) {
    throw std::runtime_error("Invalid file descriptor");
  }
}

bool UnixSocket::_isValidSocket() const noexcept {
  return _socketFd > 0;
}

UnixSocket::~UnixSocket() noexcept {
  if (_isValidSocket()) {
    close(_socketFd);
  }
}

void UnixSocket::connect(const HostData &hostData) {
  auto *addressInfo{_resolveHostDataToAddressInfo(hostData)};

  auto connectResult{
      ::connect(_socketFd, addressInfo->ai_addr, addressInfo->ai_addrlen)};

  freeaddrinfo(addressInfo);

  if (connectResult < 0) {
    throw std::runtime_error("Unable to connect socket");
  }
}

struct addrinfo *UnixSocket::_resolveHostDataToAddressInfo(
    const HostData &hostData) {
  struct addrinfo *server = nullptr;
  struct addrinfo hints{};

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  std::string portAsString{std::to_string(hostData.port)};

  auto result{getaddrinfo(hostData.host.c_str(), portAsString.c_str(), &hints,
                          &server)};

  if (result != 0) {
    throw std::runtime_error("Unable to resolve this host and port: " +
                             std::string(::gai_strerror(result)));
  }

  return server;
}

void UnixSocket::bind(std::uint16_t port) {
  auto address{_buildLocalAddressByPort(port)};

  auto bindResult{::bind(
      _socketFd, reinterpret_cast<struct sockaddr *>(&address),  // NOLINT
      sizeof(address))};

  if (bindResult < 0) {
    throw std::runtime_error("Unable to bind socket");
  }
}

struct sockaddr_in UnixSocket::_buildLocalAddressByPort(std::uint16_t port) {
  struct sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  return address;
}

std::unique_ptr<ISocket> UnixSocket::accept() {
  auto clientSocket{::accept(_socketFd, nullptr, nullptr)};

  if (clientSocket < 0) {
    throw std::runtime_error("Error while accepting socket");
  }

  return std::unique_ptr<UnixSocket>(new UnixSocket(clientSocket));
}

void UnixSocket::listen() {
  auto listenResult{::listen(_socketFd, SOMAXCONN)};

  if (listenResult < 0) {
    throw std::runtime_error("Listen failed");
  }
}

void UnixSocket::send(const std::string &data) {
  auto bytesSent{::send(_socketFd, data.data(), data.size(), 0)};

  if (bytesSent != data.size()) {
    throw std::runtime_error("Bytes sent don't match data size");
  }
}

std::string UnixSocket::receive() {
  std::string received;
  std::array<char, BUFFER_SIZE> buffer{};

  ssize_t bytesReceived{0};
  while ((bytesReceived = ::recv(_socketFd, buffer.data(), buffer.size(), 0)) >
         0) {
    received.append(buffer.data(), bytesReceived);

    if (received.find("\r\n\r\n") != std::string::npos) {
      break;
    }
  }

  return received;
}

}  // namespace webserver::net
