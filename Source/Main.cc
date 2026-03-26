#include <print>

#include "Config.h"
#include "EventsManager.h"
#include "HttpServer.h"
#include "StaticFileHandler.h"

int main() {
  using namespace webserver;

  core::EventsManager::enableEventsHandlers();

  try {
    constexpr auto kDefaultConfigFile{"config.ini"};
    config::Config serverConfig{kDefaultConfigFile};
    const http::StaticFileHandler handler{serverConfig.contentDirectory};
    net::HttpServer server{std::move(serverConfig), handler};
    server.startServerLoop();
  } catch (const std::exception &e) {
    std::println("Error: {}", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
