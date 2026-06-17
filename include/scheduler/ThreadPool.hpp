#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace scheduler {

class ThreadPool {
public:
    explicit ThreadPool(std::size_t worker_count);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template <typename Function>
    auto enqueue(Function&& function) -> std::future<std::invoke_result_t<Function>> {
        using Result = std::invoke_result_t<Function>;

        auto task = std::make_shared<std::packaged_task<Result()>>(std::forward<Function>(function));
        std::future<Result> future = task->get_future();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopping_) {
                throw std::runtime_error("Cannot enqueue work after thread pool has stopped");
            }

            tasks_.push([task]() {
                (*task)();
            });
        }

        task_available_.notify_one();
        return future;
    }

private:
    void workerLoop();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable task_available_;
    bool stopping_ = false;
};

} // namespace scheduler
