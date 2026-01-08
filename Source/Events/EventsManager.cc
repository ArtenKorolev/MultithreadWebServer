#include "EventsManager.h"

#include <unistd.h>

#include <csignal>

#include "HttpServer.h"

namespace webserver::core {

void EventsManager::enableEventsHandlers() {
#ifndef _POSIX_VERSION
#  error "Unsupported platform for events handling"
#endif
  _enableShutdownHandler();
}

void EventsManager::_enableShutdownHandler() {
  struct sigaction signalData = {};
  signalData.sa_flags = 0;

  signalData.sa_handler = []([[maybe_unused]] const int sigNum) {
    shutdownRequested.store(true);
  };

  sigemptyset(&signalData.sa_mask);
  sigaction(SIGINT, &signalData, nullptr);
}

}  // namespace webserver::core
