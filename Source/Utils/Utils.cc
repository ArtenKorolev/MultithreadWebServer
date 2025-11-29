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

std::string trim(std::string str) {
  str.erase(str.begin(),
            std::find_if(str.begin(), str.end(),
                         [](unsigned char ch) { return !std::isspace(ch); }));
  str.erase(std::find_if(str.rbegin(), str.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            str.end());
  return str;
}

}  // namespace webserver::utils
