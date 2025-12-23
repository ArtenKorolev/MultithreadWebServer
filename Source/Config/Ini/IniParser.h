#pragma once

#include <optional>
#include <string>
#include <unordered_map>

namespace webserver::core {

using UmapStrStr = std::unordered_map<std::string, std::string>;

struct IniParsingContext {
  UmapStrStr configMap;
  std::optional<std::string> currentSection{std::nullopt};
};

class IniParser {
 public:
  explicit IniParser(std::string rawConfig) : _input{std::move(rawConfig)} {
  }

  [[nodiscard]] UmapStrStr parse();

 private:
  [[nodiscard]] std::size_t _calculateLineEnd(std::size_t lineStart);
  void _parseLine(const std::string &line);
  void _parseSection(const std::string &line);
  static void _throwIfEndOfSectionIsInvalid(std::size_t endOfSection,
                                            const std::string &line);
  void _parseKeyValuePair(const std::string &line);

  std::string _input;
  IniParsingContext _parsingContext;
};

}  // namespace webserver::core
