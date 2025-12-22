#pragma once

#include <string>

namespace webserver::utils {

[[nodiscard]] std::string getCurrentDate();
[[nodiscard]] std::string trim(std::string str);
[[nodiscard]] int getNativeThreadsCount() noexcept;

}  // namespace webserver::utils
