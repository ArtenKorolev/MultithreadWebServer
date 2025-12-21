#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

#include "Config.h"

namespace webserver::core {

inline std::size_t getDefaultThreadsCount() noexcept {
  const auto count = std::thread::hardware_concurrency();
  return count == 0 ? 4 : count;
}

const auto kDefaultThreadsCount = getDefaultThreadsCount();

class ThreadPool {
 public:
  explicit ThreadPool(
      std::size_t threadsCount = config::Config::getInstance().threadsCount);

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ThreadPool &operator=(ThreadPool &&) = delete;
  ~ThreadPool();

  template <class F, class... Args>
  auto enqueue(F &&func, Args &&...args)
      -> std::future<std::invoke_result_t<F, Args...>> {
    using returnType = std::invoke_result_t<F, Args...>;

    auto taskPtr = std::make_shared<std::packaged_task<returnType()>>(
        std::bind(std::forward<F>(func), std::forward<Args>(args)...));

    {
      std::lock_guard<std::mutex> lock(_queueMutex);

      if (_stop) {
        throw std::runtime_error("Enqueue on stopped ThreadPool");
      }

      _tasks.emplace([taskPtr]() { (*taskPtr)(); });
    }

    _condition.notify_one();
    return taskPtr->get_future();
  }

  void stop();

 private:
  void _worker();

  std::vector<std::thread> _workers;
  std::queue<std::function<void()>> _tasks;

  std::mutex _queueMutex;
  std::condition_variable _condition;
  bool _stop = false;
};

}  // namespace webserver::core
