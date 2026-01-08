#include <print>

#include "Config.h"
#include "EventsManager.h"
#include "HttpServer.h"

int main() {
  using namespace webserver;

  core::EventsManager::enableEventsHandlers();

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
