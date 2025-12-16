#include <print>

#include "HttpServer.h"
#include "SignalsManager.h"

int main() {
  webserver::core::SignalsManager::enableAllSignalsHandlers();

  try {
    webserver::net::HttpServer server;
    server.startServerLoop();
  } catch (const std::exception &e) {
    std::println("Error: {}", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
