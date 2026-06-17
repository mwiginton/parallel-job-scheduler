#include "scheduler/JobConfigLoader.hpp"
#include "scheduler/JobPlanner.hpp"
#include "scheduler/ParallelJobExecutor.hpp"
#include "scheduler/ThreadPool.hpp"

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <future>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testOrdersJobsByDependencies() {
    const std::vector<scheduler::Job> jobs{
        {"package", "", {"build", "test"}},
        {"test", "", {"build"}},
        {"build", "", {"prepare"}},
        {"prepare", "", {}}
    };

    const scheduler::JobPlanner planner;
    const std::vector<const scheduler::Job*> ordered_jobs = planner.buildExecutionOrder(jobs);

    require(ordered_jobs.size() == 4, "Expected four ordered jobs");
    require(ordered_jobs[0]->name == "prepare", "Expected prepare to run first");
    require(ordered_jobs[1]->name == "build", "Expected build to run second");
    require(ordered_jobs[2]->name == "test", "Expected test to run third");
    require(ordered_jobs[3]->name == "package", "Expected package to run last");
}

void testRejectsUnknownDependency() {
    const std::vector<scheduler::Job> jobs{
        {"build", "", {"prepare"}}
    };

    const scheduler::JobPlanner planner;

    try {
        (void)planner.buildExecutionOrder(jobs);
    } catch (const std::runtime_error&) {
        return;
    }

    throw std::runtime_error("Expected unknown dependency to throw");
}

void testRejectsDuplicateJobNames() {
    const std::vector<scheduler::Job> jobs{
        {"build", "", {}},
        {"build", "", {}}
    };

    const scheduler::JobPlanner planner;

    try {
        (void)planner.buildExecutionOrder(jobs);
    } catch (const std::runtime_error&) {
        return;
    }

    throw std::runtime_error("Expected duplicate job names to throw");
}

void testDetectsCycles() {
    const std::vector<scheduler::Job> jobs{
        {"a", "", {"b"}},
        {"b", "", {"c"}},
        {"c", "", {"a"}}
    };

    const scheduler::JobPlanner planner;

    try {
        (void)planner.buildExecutionOrder(jobs);
    } catch (const std::runtime_error&) {
        return;
    }

    throw std::runtime_error("Expected dependency cycle to throw");
}

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file) {
        throw std::runtime_error("Could not write test file: " + path);
    }

    file << content;
}

void testLoadsJobsFromJsonFile() {
    const std::string path = "valid_jobs_config_test.json";
    writeFile(path, R"({
        "jobs": [
            {
                "name": "build",
                "command": "echo Building",
                "dependencies": ["prepare"]
            },
            {
                "name": "prepare",
                "command": "echo Preparing"
            }
        ]
    })");

    const scheduler::JobConfigLoader loader;
    const std::vector<scheduler::Job> jobs = loader.loadJobsFromFile(path);

    require(jobs.size() == 2, "Expected two jobs from JSON config");
    require(jobs[0].name == "build", "Expected first job name from JSON config");
    require(jobs[0].command == "echo Building", "Expected first job command from JSON config");
    require(jobs[0].dependencies.size() == 1, "Expected first job dependency from JSON config");
    require(jobs[0].dependencies[0] == "prepare", "Expected dependency name from JSON config");
    require(jobs[1].dependencies.empty(), "Expected missing dependencies field to mean no dependencies");
}

void testRejectsInvalidJsonConfig() {
    const std::string path = "invalid_jobs_config_test.json";
    writeFile(path, R"({
        "jobs": [
            {
                "name": "build",
                "dependencies": []
            }
        ]
    })");

    const scheduler::JobConfigLoader loader;

    try {
        (void)loader.loadJobsFromFile(path);
    } catch (const std::runtime_error&) {
        return;
    }

    throw std::runtime_error("Expected invalid JSON config to throw");
}

void testThreadPoolReturnsTaskResults() {
    scheduler::ThreadPool thread_pool(2);

    std::future<int> first = thread_pool.enqueue([]() {
        return 20;
    });
    std::future<int> second = thread_pool.enqueue([]() {
        return 22;
    });

    require(first.get() + second.get() == 42, "Expected thread pool task results");
}

void testThreadPoolRunsTasksConcurrently() {
    scheduler::ThreadPool thread_pool(2);

    std::mutex mutex;
    std::condition_variable ready_changed;
    std::condition_variable start_changed;
    int ready_count = 0;
    bool start = false;

    auto blocking_task = [&]() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            ++ready_count;
            ready_changed.notify_one();

            start_changed.wait(lock, [&start]() {
                return start;
            });
        }

        return 1;
    };

    std::future<int> first = thread_pool.enqueue(blocking_task);
    std::future<int> second = thread_pool.enqueue(blocking_task);

    {
        std::unique_lock<std::mutex> lock(mutex);
        const bool both_tasks_started = ready_changed.wait_for(lock, std::chrono::seconds(2), [&ready_count]() {
            return ready_count == 2;
        });
        require(both_tasks_started, "Expected two thread pool tasks to run concurrently");

        start = true;
    }

    start_changed.notify_all();

    require(first.get() == 1, "Expected first blocking task result");
    require(second.get() == 1, "Expected second blocking task result");
}

void testParallelExecutorRunsDependencyGraph() {
    const std::vector<scheduler::Job> jobs{
        {"finish", "echo finish", {"left", "right"}},
        {"left", "echo left", {}},
        {"right", "echo right", {}}
    };

    const scheduler::ParallelJobExecutor executor(2);
    require(executor.run(jobs) == 0, "Expected parallel executor to complete dependency graph");
}

} // namespace

int main() {
    try {
        testOrdersJobsByDependencies();
        testRejectsUnknownDependency();
        testRejectsDuplicateJobNames();
        testDetectsCycles();
        testLoadsJobsFromJsonFile();
        testRejectsInvalidJsonConfig();
        testThreadPoolReturnsTaskResults();
        testThreadPoolRunsTasksConcurrently();
        testParallelExecutorRunsDependencyGraph();
    } catch (const std::exception& error) {
        std::cerr << "JobPlanner test failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All JobPlanner tests passed.\n";
    return 0;
}
