#include "scheduler/Job.hpp"
#include "scheduler/JobPlanner.hpp"
#include "scheduler/JobRunner.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>

int main() {
    const std::vector<scheduler::Job> jobs{
        {
            "package",
            "echo Packaging final output",
            {"build", "test"}
        },
        {
            "test",
            "echo Running tests",
            {"build"}
        },
        {
            "build",
            "echo Building project artifacts",
            {"prepare"}
        },
        {
            "prepare",
            "echo Preparing job inputs",
            {}
        }
    };

    scheduler::JobPlanner planner;
    const scheduler::JobRunner runner;

    try {
        const std::vector<const scheduler::Job*> ordered_jobs = planner.buildExecutionOrder(jobs);

        for (const scheduler::Job* job : ordered_jobs) {
            const int exit_code = runner.run(*job);
            if (exit_code != 0) {
                std::cerr << "Stopping because job failed: " << job->name << '\n';
                return exit_code;
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Failed to plan jobs: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All jobs completed successfully.\n";
    return 0;
}
