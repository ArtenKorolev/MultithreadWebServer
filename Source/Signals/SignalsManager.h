#pragma once

namespace webserver::core {

class SignalsManager {
 public:
  static void enableAllSignalsHandlers();

 private:
  static void _enableSigIntHandler();
};

}  // namespace webserver::core
