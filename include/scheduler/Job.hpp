#pragma once

#include <string>

namespace scheduler {

struct Job {
    std::string name;
    std::string command;
};

} // namespace scheduler
