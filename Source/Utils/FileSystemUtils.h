#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace webserver::utils {

[[nodiscard]] std::optional<std::string> readFile(
    const std::filesystem::path &fileName);

}  // namespace webserver::utils
