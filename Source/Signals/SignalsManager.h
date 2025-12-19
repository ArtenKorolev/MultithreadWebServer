#pragma once

#include <unistd.h>

#if !defined(_POSIX_VERSION)
  #error "Unsupported platform for signals"
#endif

namespace webserver::core {

class SignalsManager {
 public:
  static void enableAllSignalsHandlers();

 private:
  static void _enableSigIntHandler();
};

}  // namespace webserver::core
