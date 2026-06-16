#include "scheduler/JobPlanner.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace scheduler {
namespace {

enum class VisitState {
    Visiting,
    Visited
};

struct PlanningContext {
    std::unordered_map<std::string, const Job*> jobs_by_name;
    std::unordered_map<std::string, VisitState> visit_states;
    std::vector<std::string> visit_path;
    std::vector<const Job*> ordered_jobs;
};

std::string formatCycle(const std::vector<std::string>& path, const std::string& repeated_name) {
    const auto cycle_start = std::find(path.begin(), path.end(), repeated_name);

    std::string cycle;
    for (auto it = cycle_start; it != path.end(); ++it) {
        if (!cycle.empty()) {
            cycle += " -> ";
        }
        cycle += *it;
    }

    if (!cycle.empty()) {
        cycle += " -> ";
    }
    cycle += repeated_name;

    return cycle;
}

void visitJob(const Job& job, PlanningContext& context) {
    const auto state = context.visit_states.find(job.name);
    if (state != context.visit_states.end()) {
        if (state->second == VisitState::Visiting) {
            throw std::runtime_error("Dependency cycle detected: " + formatCycle(context.visit_path, job.name));
        }

        return;
    }

    context.visit_states.emplace(job.name, VisitState::Visiting);
    context.visit_path.push_back(job.name);

    for (const std::string& dependency_name : job.dependencies) {
        const auto dependency = context.jobs_by_name.find(dependency_name);
        if (dependency == context.jobs_by_name.end()) {
            throw std::runtime_error(
                "Job '" + job.name + "' depends on unknown job '" + dependency_name + "'"
            );
        }

        visitJob(*dependency->second, context);
    }

    context.visit_path.pop_back();
    context.visit_states[job.name] = VisitState::Visited;
    context.ordered_jobs.push_back(&job);
}

} // namespace

std::vector<const Job*> JobPlanner::buildExecutionOrder(const std::vector<Job>& jobs) const {
    PlanningContext context;

    for (const Job& job : jobs) {
        const auto [_, inserted] = context.jobs_by_name.emplace(job.name, &job);
        if (!inserted) {
            throw std::runtime_error("Duplicate job name: " + job.name);
        }
    }

    for (const Job& job : jobs) {
        visitJob(job, context);
    }

    return context.ordered_jobs;
}

} // namespace scheduler
