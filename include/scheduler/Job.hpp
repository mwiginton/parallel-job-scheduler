#pragma once

#include <string>
#include <vector>

namespace scheduler {

struct Job {
    std::string name;
    std::string command;
    std::vector<std::string> dependencies;
};

} // namespace scheduler
