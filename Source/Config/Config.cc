#include "Config.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "IniParser.h"

namespace webserver::config {

Config::Config(const std::filesystem::path& configPath) {
  auto configContents{_readFile(configPath)};

  if (!configContents.has_value()) {
    return;
  }

  core::IniParser iniParser{std::move(configContents.value())};
  const auto configMap{iniParser.parse()};

  constexpr auto kPortKey{"server.port"};
  constexpr auto kWorkersKey{"server.workers"};
  constexpr auto kContentDirectoryKey{"server.content_dir"};

  if (configMap.contains(kPortKey)) {
    port = std::stoi(configMap.at(kPortKey));
  }
  if (configMap.contains(kWorkersKey)) {
    threadsCount =
        static_cast<std::uint16_t>(std::stoull(configMap.at(kWorkersKey)));
  }
  if (configMap.contains(kContentDirectoryKey)) {
    contentDirectory = configMap.at(kContentDirectoryKey);
  }
};

[[nodiscard]] std::optional<std::string> Config::_readFile(
    const std::filesystem::path& path) {
  std::ifstream fileStream{path};

  if (!fileStream.is_open()) {
    return std::nullopt;
  }

  std::stringstream strStream;
  strStream << fileStream.rdbuf();

  return std::optional<std::string>{std::in_place_t{}, strStream.str()};
};

}  // namespace webserver::config
