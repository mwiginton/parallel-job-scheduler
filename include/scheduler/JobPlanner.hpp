#pragma once

#include "scheduler/Job.hpp"

#include <vector>

namespace scheduler {

class JobPlanner {
public:
    std::vector<const Job*> buildExecutionOrder(const std::vector<Job>& jobs) const;
};

} // namespace scheduler
