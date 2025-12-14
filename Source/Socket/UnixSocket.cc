#include "UnixSocket.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <filesystem>
#include <memory>
#include <stdexcept>

#include "HostData.h"

namespace webserver::net {

constexpr auto kBufferSize = 1024;

UnixSocket::UnixSocket() {
  const auto fileDescriptor = ::socket(AF_INET, SOCK_STREAM, 0);

  if (!_isValidFileDescriptor(fileDescriptor)) {
    throw std::runtime_error("Unable to initialize socket");
  }

  constexpr auto enable = 1;
  setsockopt(fileDescriptor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

  _socketFd = fileDescriptor;
}

UnixSocket::UnixSocket(const int fileDescriptor) {
  if (!_isValidFileDescriptor(fileDescriptor)) {
    throw std::runtime_error("Invalid file descriptor");
  }

  _socketFd = fileDescriptor;
}

bool UnixSocket::_isValidFileDescriptor(const int fileDescriptor) noexcept {
  return fileDescriptor >= 0;
}

UnixSocket::~UnixSocket() noexcept {
  if (_isValidFileDescriptor(_socketFd)) {
    close(_socketFd);
  }
}

void UnixSocket::connect(const HostData &hostData) {
  auto *addressInfo{_resolveHostDataToAddressInfo(hostData)};

  const auto connectResult{
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

  const std::string portAsString{std::to_string(hostData.port)};

  const auto result{getaddrinfo(hostData.host.c_str(), portAsString.c_str(),
                                &hints, &server)};

  if (result != 0) {
    throw std::runtime_error("Unable to resolve this host and port: " +
                             std::string(::gai_strerror(result)));
  }

  return server;
}

void UnixSocket::bind(const std::uint16_t port) {
  auto address{_buildLocalAddressByPort(port)};

  const auto bindResult{::bind(
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
  const auto clientFileDescriptor{::accept(_socketFd, nullptr, nullptr)};

  if (!_isValidFileDescriptor(clientFileDescriptor)) {
    throw std::runtime_error("Error while accepting socket");
  }

  return std::unique_ptr<UnixSocket>(new UnixSocket(clientFileDescriptor));
}

void UnixSocket::listen() {
  if (const auto listenResult{::listen(_socketFd, SOMAXCONN)};
      listenResult < 0) {
    throw std::runtime_error("Listen failed");
  }
}

void UnixSocket::send(const std::string &data) {
  if (const auto bytesSent{::send(_socketFd, data.data(), data.size(), 0)};
      bytesSent != data.size()) {
    throw std::runtime_error("Bytes sent don't match data size");
  }
}

std::string UnixSocket::receive() {
  std::string received;
  std::array<char, kBufferSize> buffer{};

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

void UnixSocket::sendZeroCopyFile(const std::filesystem::path filePath) {
  if (filePath.empty()) {
    throw std::runtime_error("Empty file path");
  }

  const int fileFd = open(filePath.c_str(), O_RDONLY);
  if (fileFd < 0) {
    throw std::runtime_error("Failed to open file");
  }

  struct stat stats{};
  if (fstat(fileFd, &stats) < 0) {
    close(fileFd);
    throw std::runtime_error("Failed to stat file");
  }

  off_t offset = 0;
  off_t remaining = stats.st_size;

#if defined(__APPLE__) && defined(__MACH__)
  while (remaining > 0) {
    off_t toSend = remaining;

    const int result = sendfile(fileFd, _socketFd, offset, &toSend, nullptr, 0);

    if (result < 0) {
      close(fileFd);
      throw std::runtime_error("sendfile() failed");
    }

    offset += toSend;
    remaining -= toSend;
  }

#elif defined(__linux__)
  while (remaining > 0) {
    const ssize_t sent = sendfile(_socketFd, fileFd, &offset, remaining);

    if (sent == -1) {
      close(fileFd);
      throw std::runtime_error("sendfile() failed");
    }

    remaining -= sent;
  }
#else
  #error "Unsupported UNIX platform"
#endif

  close(fileFd);
}

}  // namespace webserver::net
