#pragma once

#include "scheduler/Job.hpp"

namespace scheduler {

class JobRunner {
public:
    int run(const Job& job) const;
};

} // namespace scheduler
