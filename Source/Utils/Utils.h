#pragma once

#include <string>

namespace webserver::utils {

[[nodiscard]] std::string getCurrentDate();

[[nodiscard]] std::string trim(std::string str);

}  // namespace webserver::utils
