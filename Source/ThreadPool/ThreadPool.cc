#include "ThreadPool.h"

#include <mutex>

namespace webserver::core {

ThreadPool::ThreadPool(std::size_t threadsCount) {
  for (std::size_t i = 0; i < threadsCount; ++i) {
    _workers.emplace_back([this] { this->_worker(); });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::lock_guard<std::mutex> lock{_queueMutex};
    _stop = true;
  }

  _condition.notify_all();

  for (auto &worker : _workers) {
    worker.join();
  }
}

void ThreadPool::_worker() {
  while (true) {
    std::function<void()> task;

    {
      std::unique_lock lock{_queueMutex};
      _condition.wait(lock, [this] { return _stop || !_tasks.empty(); });

      if (_stop && _tasks.empty()) {
        return;
      }

      task = std::move(_tasks.front());
      _tasks.pop();
    }

    task();
  }
}

}  // namespace webserver::core
