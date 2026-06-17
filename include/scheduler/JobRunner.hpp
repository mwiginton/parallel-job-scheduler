#pragma once

#include "scheduler/Job.hpp"

#include <mutex>

namespace scheduler {

class JobRunner {
public:
    int run(const Job& job) const;

private:
    mutable std::mutex output_mutex_;
};

} // namespace scheduler
