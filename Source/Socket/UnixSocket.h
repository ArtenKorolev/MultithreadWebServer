#pragma once

#include <netdb.h>

#include "HostData.h"
#include "Socket.h"

namespace webserver::net {

class UnixSocket final : public ISocket {
 public:
  UnixSocket();
  UnixSocket(const UnixSocket &) = delete;
  UnixSocket(UnixSocket &&) = delete;
  UnixSocket &operator=(const UnixSocket &) = delete;
  UnixSocket &operator=(UnixSocket &&) = delete;
  ~UnixSocket() noexcept override;

  void connect(const HostData &hostData) override;
  void bind(std::uint16_t port) override;
  std::unique_ptr<ISocket> accept() override;
  void listen() override;
  void send(const std::string &data) override;
  std::string receive() override;

 private:
  explicit UnixSocket(int fileDescriptor);

  [[nodiscard]] static bool _isValidFileDescriptor(
      int fileDescriptor) noexcept;
  [[nodiscard]] static struct addrinfo *_resolveHostDataToAddressInfo(
      const HostData &hostData);
  [[nodiscard]] static struct sockaddr_in _buildLocalAddressByPort(
      std::uint16_t port);

  int _socketFd{};
};

}  // namespace webserver::net
