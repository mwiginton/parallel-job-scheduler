#include "scheduler/JobConfigLoader.hpp"
#include "scheduler/Job.hpp"
#include "scheduler/ParallelJobExecutor.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char* argv[]) {
    const std::string config_path = argc > 1 ? argv[1] : "jobs.json";
    const std::size_t worker_count = argc > 2
        ? static_cast<std::size_t>(std::stoul(argv[2]))
        : static_cast<std::size_t>(std::max(1u, std::thread::hardware_concurrency()));

    scheduler::JobConfigLoader config_loader;
    const scheduler::ParallelJobExecutor executor(worker_count);

    try {
        const std::vector<scheduler::Job> jobs = config_loader.loadJobsFromFile(config_path);
        const int exit_code = executor.run(jobs);
        if (exit_code != 0) {
            return exit_code;
        }
    } catch (const std::exception& error) {
        std::cerr << "Failed to run jobs: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All jobs completed successfully.\n";
    return 0;
}
