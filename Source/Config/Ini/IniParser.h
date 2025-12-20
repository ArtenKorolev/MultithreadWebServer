#pragma once

#include <string>
#include <unordered_map>

namespace webserver::core {

using UmapStrStr = std::unordered_map<std::string, std::string>;

class IniParser {
 public:
  explicit IniParser(std::string rawConfig) : config{std::move(rawConfig)} {
  }

  [[nodiscard]] UmapStrStr parse() const;

 private:
  std::string config;
};

}  // namespace webserver::core
