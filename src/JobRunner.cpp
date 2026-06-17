#include "scheduler/JobRunner.hpp"

#include <cstdlib>
#include <iostream>
#include <mutex>

namespace scheduler {

int JobRunner::run(const Job& job) const {
    {
        std::lock_guard<std::mutex> lock(output_mutex_);
        std::cout << "Running job: " << job.name << '\n';
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
