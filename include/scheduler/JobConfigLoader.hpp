#pragma once

#include "scheduler/Job.hpp"

#include <string>
#include <vector>

namespace scheduler {

class JobConfigLoader {
public:
    std::vector<Job> loadJobsFromFile(const std::string& path) const;
};

} // namespace scheduler
