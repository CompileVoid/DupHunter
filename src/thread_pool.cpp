#include "duphunter/thread_pool.h"

#include <cstddef>
#include <thread>

namespace duphunter {
ThreadPool::ThreadPool(size_t num_threads) {
  for (std::size_t i = 0; i < num_threads; ++i) {
    workers_.emplace_back([this](std::stop_token stoken) {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(queue_mutex_);

          condition_.wait(lock, [this, &stoken] {
            return stop_requested_ || stoken.stop_requested() ||
                   !tasks_.empty();
          });

          if ((stop_requested_ || stoken.stop_requested()) && tasks_.empty()) {
            return;
          }

          task = std::move(tasks_.front());
          tasks_.pop();
        }

        task();
      }
    });
  }
}

ThreadPool::~ThreadPool() { stop(); }

void ThreadPool::stop() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    if (stop_requested_) {
      return;
    }
    stop_requested_ = true;
  }
  condition_.notify_all();
}

} // namespace duphunter