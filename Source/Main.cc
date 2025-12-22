#include <print>

#include "Config.h"
#include "HttpServer.h"
#include "SignalsManager.h"

int main() {
  using namespace webserver;

  core::SignalsManager::enableAllSignalsHandlers();

  try {
    constexpr auto kDefaultConfigFile{"config.ini"};
    config::Config serverConfig{kDefaultConfigFile};
    net::HttpServer server{std::move(serverConfig)};
    server.startServerLoop();
  } catch (const std::exception &e) {
    std::println("Error: {}", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
