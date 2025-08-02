#include "Utils.h"

#include <chrono>
#include <iomanip>

namespace webserver::utils {

std::string getCurrentDate() {
  using namespace std::chrono;

  const auto now = system_clock::now();
  std::time_t now_time = system_clock::to_time_t(now);

  std::tm gmt{};

#if defined(_WIN32) || defined(_WIN64)
  gmtime_s(&gmt, &now_time);
#else
  gmt = *std::gmtime(&now_time);
#endif

  std::ostringstream oss;
  oss << std::put_time(&gmt, "%a, %d %b %Y %H:%M:%S GMT");
  return oss.str();
}

}  // namespace webserver::utils
