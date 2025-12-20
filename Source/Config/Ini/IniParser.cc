#include "IniParser.h"

#include <unordered_map>

#include "Utils.h"

namespace webserver::core {

UmapStrStr IniParser::parse() {
  size_t lineStart{0};

  while (true) {
    const auto lineEnd{_input.find('\n', lineStart)};

    if (lineEnd == std::string_view::npos) {
      break;
    }

    const auto lineLen{lineEnd - lineStart};
    const auto line{_input.substr(lineStart, lineLen)};

    _parseLine(line);

    if (lineEnd == _input.size() - 1) {
      break;
    }

    lineStart = lineEnd + 1;  // skip newline character
  };

  return _parsingContext.configMap;
}

void IniParser::_parseLine(const std::string &line) {
  if (line.empty()) {
    return;  // skip empty lines
  }

  if (line.at(0) == ';' || line.at(0) == '#') {
    return;  // skip comments
  }

  if (line.at(0) == '[') {
    _parseSection(line);
    return;
  }

  _parseKeyValuePair(line);
}

void IniParser::_parseSection(const std::string &line) {
  const auto lineTrimmed{utils::trim(line)};

  if (line.at(0) != '[') {
    throw std::runtime_error("not a valid section declaration");
  }

  const auto endOfSection{lineTrimmed.find(']')};

  if (endOfSection == std::string::npos) {
    throw std::runtime_error("missing ']' in section declaration");
  }

  if (endOfSection != lineTrimmed.size() - 1) {
    throw std::runtime_error("unexpected symbols after ']'");
  }

  auto section{utils::trim(lineTrimmed.substr(1, endOfSection - 1))};

  if (section.empty()) {
    throw std::runtime_error("empty section ");
  }

  _parsingContext.currentSection = std::move(section);
}

void IniParser::_parseKeyValuePair(const std::string &line) {
  std::string key;
  std::string value;
  bool eqSignMet{false};

  for (const char chr : line) {
    if (chr == '=') {
      if (eqSignMet) {
        throw std::runtime_error("double '=' in .ini file");
      }

      if (key.empty()) {
        throw std::runtime_error("key is empty");
      }

      eqSignMet = true;
    } else if (eqSignMet) {
      value += chr;
    } else {
      key += chr;
    }
  }

  if (!eqSignMet) {
    throw std::runtime_error("expected '=' in variable assignment");
  }

  if (!_parsingContext.currentSection.has_value()) {
    throw std::runtime_error("current section is not defined");
  }

  auto keyWithSection{_parsingContext.currentSection.value() + "." +
                      utils::trim(std::move(key))};

  _parsingContext.configMap[std::move(keyWithSection)] =
      utils::trim(std::move(value));
}

}  // namespace webserver::core
