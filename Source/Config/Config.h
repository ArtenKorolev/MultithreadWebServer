#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>

#include "IniParser.h"

constexpr auto kDefaultPort = 8000;

namespace webserver::config {

class Config {
 public:
  static Config getInstance() {
    static auto instance = _fromFile("config.ini");
    return instance;
  }

  int port{kDefaultPort};
  std::size_t threadsCount{2};
  std::string contentDirectory;

 private:
  static Config _fromFile(const std::filesystem::path& path) {
    auto configContents{_readFile(path)};
    core::IniParser iniParser{std::move(configContents)};
    const auto configMap{iniParser.parse()};

    const Config newConfig{
        .port = std::stoi(configMap.at("server.port")),
        .threadsCount = std::stoull(configMap.at("server.workers")),
        .contentDirectory = configMap.at("server.content_dir")};

    return newConfig;
  }

  [[nodiscard]] static std::string _readFile(
      const std::filesystem::path& path) {
    std::ifstream fileStream{path};

    if (!fileStream.is_open()) {
      throw std::runtime_error{"Cannot open file"};
    }

    std::stringstream strStream;
    strStream << fileStream.rdbuf();

    return strStream.str();
  }
};

}  // namespace webserver::config
