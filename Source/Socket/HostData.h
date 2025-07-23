#pragma once

#include <string>

namespace webserver::net {

struct HostData {
  HostData() = default;
  HostData(std::string host, std::uint16_t port)
      : host{std::move(host)}, port{port} {
  }

  std::string host;
  std::uint16_t port{};
};

}  // namespace webserver::net
