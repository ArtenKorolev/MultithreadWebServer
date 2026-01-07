#include "Config.h"

#include <filesystem>

#include "FileSystemUtils.h"
#include "IniParser.h"

namespace webserver::config {

Config::Config(const std::filesystem::path& configPath) {
  auto configContents{utils::readFile(configPath)};

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

}  // namespace webserver::config
