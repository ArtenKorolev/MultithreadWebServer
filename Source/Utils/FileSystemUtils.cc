#include "FileSystemUtils.h"

#include <fstream>

namespace webserver::utils {

[[nodiscard]] std::optional<std::string> readFile(
    const std::filesystem::path &fileName) {
  std::ifstream stream(fileName);

  if (!stream.is_open()) {
    return std::nullopt;
  }

  return std::optional<std::string>{{std::istreambuf_iterator<char>(stream),
                                     std::istreambuf_iterator<char>()}};
}

}  // namespace webserver::utils
