#include "scheduler/ThreadPool.hpp"

#include <algorithm>

namespace scheduler {

ThreadPool::ThreadPool(std::size_t worker_count) {
    const std::size_t actual_worker_count = std::max<std::size_t>(1, worker_count);
    workers_.reserve(actual_worker_count);

    for (std::size_t index = 0; index < actual_worker_count; ++index) {
        workers_.emplace_back([this]() {
            workerLoop();
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopping_ = true;
    }

    task_available_.notify_all();

    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            task_available_.wait(lock, [this]() {
                return stopping_ || !tasks_.empty();
            });

            if (stopping_ && tasks_.empty()) {
                return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
        }

        task();
    }
}

} // namespace scheduler
