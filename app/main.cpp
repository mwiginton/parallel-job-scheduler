#include "scheduler/JobConfigLoader.hpp"
#include "scheduler/Job.hpp"
#include "scheduler/JobPlanner.hpp"
#include "scheduler/JobRunner.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    const std::string config_path = argc > 1 ? argv[1] : "jobs.json";

    scheduler::JobConfigLoader config_loader;
    scheduler::JobPlanner planner;
    const scheduler::JobRunner runner;

    try {
        const std::vector<scheduler::Job> jobs = config_loader.loadJobsFromFile(config_path);
        const std::vector<const scheduler::Job*> ordered_jobs = planner.buildExecutionOrder(jobs);

        for (const scheduler::Job* job : ordered_jobs) {
            const int exit_code = runner.run(*job);
            if (exit_code != 0) {
                std::cerr << "Stopping because job failed: " << job->name << '\n';
                return exit_code;
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Failed to run jobs: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All jobs completed successfully.\n";
    return 0;
}
