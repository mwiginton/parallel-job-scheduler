#include "scheduler/JobRunner.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>

namespace scheduler {

int JobRunner::run(const Job& job) const {
    const std::size_t max_attempts = job.max_retries + 1;

    for (std::size_t attempt = 1; attempt <= max_attempts; ++attempt) {
        const int exit_code = runOnce(job, attempt, max_attempts);
        if (exit_code == 0 || attempt == max_attempts) {
            return exit_code;
        }

        const auto backoff = job.retry_backoff * static_cast<int>(attempt);

        {
            std::lock_guard<std::mutex> lock(output_mutex_);
            std::cout << "Retrying job '" << job.name << "' in "
                      << backoff.count() << " ms.\n";
        }

        std::this_thread::sleep_for(backoff);
    }

    return 1;
}

int JobRunner::runOnce(const Job& job, std::size_t attempt, std::size_t max_attempts) const {
    {
        std::lock_guard<std::mutex> lock(output_mutex_);
        std::cout << "Running job: " << job.name << '\n';
        if (max_attempts > 1) {
            std::cout << "Attempt: " << attempt << " of " << max_attempts << '\n';
        }
        std::cout << "Command: " << job.command << '\n';
        std::cout << std::flush;
    }

    const int exit_code = std::system(job.command.c_str());

    {
        std::lock_guard<std::mutex> lock(output_mutex_);
        std::cout << "Job exited with code: " << exit_code << '\n';
    }

    return exit_code;
}

} // namespace scheduler
