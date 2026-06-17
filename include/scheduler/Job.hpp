#pragma once

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

namespace scheduler {

struct Job {
    std::string name;
    std::string command;
    std::vector<std::string> dependencies;
    std::size_t max_retries = 0;
    std::chrono::milliseconds retry_backoff{0};
};

} // namespace scheduler
