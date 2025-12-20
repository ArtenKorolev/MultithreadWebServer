#include "IniParser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "Utils.h"

namespace webserver::core {

void parse_line(UmapStrStr &, const std::string_view,
                const std::optional<std::string>);
std::string parse_section_declaration(const std::string_view line);

UmapStrStr IniParser::parse() const {
  UmapStrStr map;

  const std::string_view config_sv = config;
  std::optional<std::string> cur_section = std::nullopt;
  auto start_of_line = 0;

  while (true) {
    auto end_of_line = config_sv.find('\n', start_of_line);

    if (end_of_line == std::string_view::npos) {
      end_of_line = config_sv.size() - 1;
    }

    const auto line_len = end_of_line - start_of_line;
    const auto line = config_sv.substr(start_of_line, line_len);
    std::cout << "[parser] line: " << line << '\n';

    if (!line.empty() && line[0] == '[') {
      cur_section = parse_section_declaration(line);
    } else {
      parse_line(map, line, cur_section);
    }

    if (end_of_line == config_sv.size() - 1) {
      break;
    }

    start_of_line = end_of_line + 1;  // skip new line character
  };

  return map;
}

std::string parse_section_declaration(const std::string_view line) {
  const auto line_trimmed = utils::trim(std::string(line));

  if (line[0] != '[') {
    throw std::runtime_error("[parser] not a valid section declaration");
  }

  const auto end_of_section_decl = line_trimmed.find(']');

  if (end_of_section_decl == std::string::npos) {
    throw std::runtime_error("[parser] missing ']' in section declaration");
  }

  if (end_of_section_decl != line_trimmed.size() - 1) {
    throw std::runtime_error("[parser] unexpected symbols after ']'");
  }

  const auto section =
      utils::trim(line_trimmed.substr(1, end_of_section_decl - 1));

  if (section.empty()) {
    throw std::runtime_error("[parser] empty section declaration");
  }

  std::cout << "[parser] section declaration found, section: " << section
            << '\n';

  return section;
}

void parse_line(UmapStrStr &map, const std::string_view line,
                const std::optional<std::string> cur_section) {
  if (line.empty()) {
    std::cout << "[parser] empty line\n";
    return;  // skip empty lines
  }

  if (line[0] == ';' || line[0] == '#') {
    std::cout << "[parser] comment: " << line << '\n';
    return;  // skip comments
  }

  std::string key, value;
  bool eq_met = false;

  for (int i = 0; i < line.size(); ++i) {
    const char c = line[i];

    if (c == '=') {
      if (eq_met) {
        throw std::runtime_error("[parser] double '=' in .ini file");
      }

      eq_met = true;
    } else if (eq_met) {
      value += c;
    } else if (!eq_met) {
      key += c;
    }
  }

  if (!eq_met) {
    throw std::runtime_error("[parser] expected '=' in variable assignment");
  }

  if (key.empty()) {
    throw std::runtime_error("[parser] key is empty");
  }

  if (!cur_section.has_value()) {
    throw std::runtime_error("[parser] current section ");
  }

  map[cur_section.value() + "." + utils::trim(std::move(key))] =
      utils::trim(std::move(value));
}

void debug_config_result(const UmapStrStr &result) {
  if (result.empty()) {
    std::cout << "[parser] empty config\n";
  }

  for (const auto kv : result) {
    std::cout << "[config] key: " << kv.first << "; value: " << kv.second
              << '\n';
  }
}

}  // namespace webserver::core
