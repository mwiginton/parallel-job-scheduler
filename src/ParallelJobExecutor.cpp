#include "scheduler/ParallelJobExecutor.hpp"

#include "scheduler/JobPlanner.hpp"
#include "scheduler/JobRunner.hpp"
#include "scheduler/ThreadPool.hpp"

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace scheduler {
namespace {

struct CompletedJob {
    const Job* job;
    int exit_code;
};

} // namespace

ParallelJobExecutor::ParallelJobExecutor(std::size_t worker_count)
    : worker_count_(worker_count == 0 ? 1 : worker_count) {}

int ParallelJobExecutor::run(const std::vector<Job>& jobs) const {
    JobPlanner planner;
    (void)planner.buildExecutionOrder(jobs);

    if (jobs.empty()) {
        std::cout << "No jobs to run.\n";
        return 0;
    }

    std::unordered_map<std::string, std::vector<const Job*>> dependents_by_name;
    std::unordered_map<std::string, std::size_t> remaining_dependencies_by_name;
    std::queue<const Job*> ready_jobs;

    for (const Job& job : jobs) {
        remaining_dependencies_by_name.emplace(job.name, job.dependencies.size());

        if (job.dependencies.empty()) {
            ready_jobs.push(&job);
        }
    }

    for (const Job& job : jobs) {
        for (const std::string& dependency_name : job.dependencies) {
            dependents_by_name[dependency_name].push_back(&job);
        }
    }

    std::cout << "Scheduling " << jobs.size() << " job(s) with " << worker_count_ << " worker(s).\n";

    ThreadPool thread_pool(worker_count_);
    JobRunner runner;

    std::mutex completion_mutex;
    std::condition_variable completion_available;
    std::queue<CompletedJob> completed_jobs;

    std::size_t completed_count = 0;
    std::size_t running_count = 0;
    int first_failure = 0;

    auto submit = [&](const Job* job) {
        ++running_count;
        (void)thread_pool.enqueue([&runner, &completion_mutex, &completion_available, &completed_jobs, job]() {
            int exit_code = 1;

            try {
                exit_code = runner.run(*job);
            } catch (const std::exception& error) {
                std::cerr << "Job threw an exception: " << job->name << ": " << error.what() << '\n';
            }

            {
                std::lock_guard<std::mutex> lock(completion_mutex);
                completed_jobs.push(CompletedJob{job, exit_code});
            }

            completion_available.notify_one();
        });
    };

    while (true) {
        while (first_failure == 0 && !ready_jobs.empty()) {
            const Job* job = ready_jobs.front();
            ready_jobs.pop();
            submit(job);
        }

        if (running_count == 0) {
            if (first_failure != 0) {
                return first_failure;
            }

            if (completed_count == jobs.size()) {
                return 0;
            }

            throw std::runtime_error("No runnable jobs remain before all jobs completed");
        }

        CompletedJob completed_job{};
        {
            std::unique_lock<std::mutex> lock(completion_mutex);
            completion_available.wait(lock, [&completed_jobs]() {
                return !completed_jobs.empty();
            });

            completed_job = completed_jobs.front();
            completed_jobs.pop();
        }

        --running_count;
        ++completed_count;

        if (completed_job.exit_code != 0) {
            if (first_failure == 0) {
                first_failure = completed_job.exit_code;
                std::cerr << "Stopping because job failed: " << completed_job.job->name << '\n';
            }
            continue;
        }

        if (first_failure != 0) {
            continue;
        }

        for (const Job* dependent : dependents_by_name[completed_job.job->name]) {
            std::size_t& remaining_dependencies = remaining_dependencies_by_name[dependent->name];
            --remaining_dependencies;

            if (remaining_dependencies == 0) {
                ready_jobs.push(dependent);
            }
        }
    }
}

} // namespace scheduler
