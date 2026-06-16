#include "scheduler/Job.hpp"
#include "scheduler/JobRunner.hpp"

int main() {
    const scheduler::Job job{
        "hello-job",
        "echo Hello from the hardcoded job"
    };

    const scheduler::JobRunner runner;
    return runner.run(job);
}
