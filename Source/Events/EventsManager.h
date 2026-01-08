#pragma once

namespace webserver::core {

class EventsManager {
 public:
  static void enableEventsHandlers();

 private:
  static void _enableShutdownHandler();
};

}  // namespace webserver::core
