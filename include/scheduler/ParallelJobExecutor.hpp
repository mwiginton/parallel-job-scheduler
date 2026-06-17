#pragma once

#include "scheduler/Job.hpp"

#include <cstddef>
#include <vector>

namespace scheduler {

class ParallelJobExecutor {
public:
    explicit ParallelJobExecutor(std::size_t worker_count);

    int run(const std::vector<Job>& jobs) const;

private:
    std::size_t worker_count_;
};

} // namespace scheduler
