#include "SignalsManager.h"

#include <unistd.h>

#include <csignal>

namespace webserver::core {

void SignalsManager::enableAllSignalsHandlers() {
  _enableSigIntHandler();
}

void SignalsManager::_enableSigIntHandler() {
  struct sigaction signalData = {};
  signalData.sa_flags = 0;

  signalData.sa_handler = []([[maybe_unused]] const int sigNum) {
    shutdownRequested.store(true);
  };

  sigemptyset(&signalData.sa_mask);
  sigaction(SIGINT, &signalData, nullptr);
}

}  // namespace webserver::core
