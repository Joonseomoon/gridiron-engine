cmake_minimum_required(VERSION 3.20)
project(gridiron_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG v1.18.0
)

FetchContent_MakeAvailable(googletest)
enable_testing()

# Warnings for our own targets only (not GoogleTest). CI turns them into
# errors with -DGRIDIRON_WERROR=ON; locally they stay warnings so a newer
# compiler's new warning doesn't block a build.
option(GRIDIRON_WERROR "Treat compiler warnings as errors" OFF)
add_library(gridiron_warnings INTERFACE)
target_compile_options(gridiron_warnings INTERFACE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
if (GRIDIRON_WERROR)
    target_compile_options(gridiron_warnings INTERFACE -Werror)
endif ()

add_library(gridiron_core
        src/engine/ApplyOutcome.cpp
        src/engine/OutcomeSampler.cpp
        src/engine/GameRunner.cpp
        src/engine/BatchSummary.cpp
        src/engine/SimulationResult.cpp
)
target_link_libraries(gridiron_core PRIVATE gridiron_warnings)
add_library(gridiron_cli
        src/cli/Teams.cpp
        src/cli/CommandLine.cpp
        src/cli/GameLog.cpp
        src/cli/BatchLog.cpp
)
target_link_libraries(gridiron_cli PRIVATE gridiron_warnings)

add_executable(simulate src/main.cpp)
add_executable(gridiron_tests
        tests/apply_outcome_test.cpp
        tests/outcome_sampler_test.cpp
        tests/integration_test.cpp
        tests/game_run_test.cpp
        tests/invariant_sweep_test.cpp
        tests/teams_test.cpp
        tests/command_line_test.cpp
        tests/game_log_test.cpp
        tests/batch_summary_test.cpp
        tests/batch_log_test.cpp
        tests/simulation_result_test.cpp
)
target_link_libraries(simulate PRIVATE gridiron_cli gridiron_core gridiron_warnings)
target_link_libraries(gridiron_tests PRIVATE gtest_main gridiron_cli gridiron_core gridiron_warnings)
include(GoogleTest)
gtest_discover_tests(gridiron_tests)
