#pragma once

#include <filesystem>

#include "Utils.h"

namespace webserver::config {

static constexpr auto kDefaultPort{8000};
static const auto kDefaultThreadsCount{utils::getNativeThreadsCount()};
static constexpr auto kDefaultContentDirectory{"public"};

class Config {
 public:
  explicit Config(const std::filesystem::path& configPath);

  std::uint16_t port{kDefaultPort};
  int threadsCount{kDefaultThreadsCount};
  std::string contentDirectory{kDefaultContentDirectory};
};

}  // namespace webserver::config
