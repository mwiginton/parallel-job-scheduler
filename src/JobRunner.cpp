#include "scheduler/JobRunner.hpp"

#include <cstdlib>
#include <iostream>

namespace scheduler {

int JobRunner::run(const Job& job) const {
    std::cout << "Running job: " << job.name << '\n';
    std::cout << "Command: " << job.command << '\n';
    std::cout << std::flush;

    const int exit_code = std::system(job.command.c_str());

    std::cout << "Job exited with code: " << exit_code << '\n';
    return exit_code;
}

} // namespace scheduler
