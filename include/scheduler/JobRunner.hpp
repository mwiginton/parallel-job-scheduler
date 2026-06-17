#pragma once

#include "scheduler/Job.hpp"

#include <mutex>

namespace scheduler {

class JobRunner {
public:
    int run(const Job& job) const;

private:
    int runOnce(const Job& job, std::size_t attempt, std::size_t max_attempts) const;

    mutable std::mutex output_mutex_;
};

} // namespace scheduler
