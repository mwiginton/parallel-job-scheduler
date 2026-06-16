#include "scheduler/Job.hpp"
#include "scheduler/JobRunner.hpp"

#include <iostream>
#include <vector>

int main() {
    const std::vector<scheduler::Job> jobs{
        {
            "prepare",
            "echo Preparing job inputs"
        },
        {
            "build",
            "echo Building project artifacts"
        },
        {
            "package",
            "echo Packaging final output"
        }
    };

    const scheduler::JobRunner runner;

    for (const scheduler::Job& job : jobs) {
        const int exit_code = runner.run(job);
        if (exit_code != 0) {
            std::cerr << "Stopping because job failed: " << job.name << '\n';
            return exit_code;
        }
    }

    std::cout << "All jobs completed successfully.\n";
    return 0;
}
