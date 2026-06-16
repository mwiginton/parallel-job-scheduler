#include "scheduler/JobPlanner.hpp"

#include <iostream>
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

} // namespace

int main() {
    try {
        testOrdersJobsByDependencies();
        testRejectsUnknownDependency();
        testRejectsDuplicateJobNames();
        testDetectsCycles();
    } catch (const std::exception& error) {
        std::cerr << "JobPlanner test failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All JobPlanner tests passed.\n";
    return 0;
}
