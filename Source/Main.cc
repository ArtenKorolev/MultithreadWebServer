#include <print>

#include "Config.h"
#include "HttpServer.h"
#include "SignalsManager.h"

int main() {
  using namespace webserver;

  core::SignalsManager::enableAllSignalsHandlers();

  try {
    net::HttpServer server{
        static_cast<std::uint16_t>(config::Config::getInstance().port)};
    server.startServerLoop();
  } catch (const std::exception &e) {
    std::println("Error: {}", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
