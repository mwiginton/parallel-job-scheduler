#include <cstdlib>
#include <iostream>
#include <string>

struct Job {
    std::string name;
    std::string command;
};

int main() {
    const Job job{
        "hello-job",
        "echo Hello from the hardcoded job"
    };

    std::cout << "Running job: " << job.name << '\n';
    std::cout << "Command: " << job.command << '\n';
    std::cout << std::flush;

    const int exit_code = std::system(job.command.c_str());

    std::cout << "Job exited with code: " << exit_code << '\n';
    return exit_code;
}
