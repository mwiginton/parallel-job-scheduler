cmake_minimum_required(VERSION 3.16)

project(parallel_job_scheduler VERSION 0.1.0 LANGUAGES CXX)

enable_testing()

add_library(scheduler_lib
    src/JobConfigLoader.cpp
    src/JobPlanner.cpp
    src/JobRunner.cpp
)

target_include_directories(scheduler_lib
    PUBLIC
        include
)

target_compile_features(scheduler_lib
    PUBLIC
        cxx_std_20
)

add_executable(parallel_job_scheduler
    app/main.cpp
)

target_link_libraries(parallel_job_scheduler
    PRIVATE
        scheduler_lib
)

add_executable(scheduler_tests
    tests/JobPlannerTests.cpp
)

target_link_libraries(scheduler_tests
    PRIVATE
        scheduler_lib
)

add_test(NAME scheduler_tests COMMAND scheduler_tests)
