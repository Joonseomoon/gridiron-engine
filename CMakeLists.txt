cmake_minimum_required(VERSION 3.20)
project(gridiron_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
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
target_compile_options(gridiron_warnings INTERFACE -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wimplicit-fallthrough)
if (GRIDIRON_WERROR)
    target_compile_options(gridiron_warnings INTERFACE -Werror)
endif ()

add_library(gridiron_core
        src/engine/batch/BatchRunner.cpp
        src/engine/batch/BatchSummary.cpp
        src/engine/batch/ParallelFor.cpp
        src/engine/batch/SimulationResult.cpp
        src/engine/game/GameRunner.cpp
        src/engine/game/ScenarioValidation.cpp
        src/engine/policy/AggressivePolicy.cpp
        src/engine/policy/ConservativePolicy.cpp
        src/engine/policy/DefaultPolicy.cpp
        src/engine/policy/LateGamePolicy.cpp
        src/engine/rules/ApplyOutcome.cpp
        src/engine/rules/Overtime.cpp
        src/engine/rules/Timeout.cpp
        src/engine/sampling/OutcomeSampler.cpp
)
# Includes are written from src/, e.g. "engine/core/GameState.h".
target_include_directories(gridiron_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(gridiron_core PRIVATE gridiron_warnings Threads::Threads)
add_library(gridiron_cli
        src/cli/BatchCsv.cpp
        src/cli/BatchLog.cpp
        src/cli/CommandLine.cpp
        src/cli/GameLog.cpp
        src/cli/Teams.cpp
)
target_include_directories(gridiron_cli PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(gridiron_cli PRIVATE gridiron_warnings Threads::Threads)

add_executable(simulate src/main.cpp)
add_executable(gridiron_tests
        tests/cli/batch_csv_test.cpp
        tests/cli/batch_log_test.cpp
        tests/cli/cli_export_test.cpp
        tests/cli/command_line_test.cpp
        tests/cli/game_log_test.cpp
        tests/cli/teams_test.cpp
        tests/engine/batch/batch_runner_test.cpp
        tests/engine/batch/batch_summary_test.cpp
        tests/engine/batch/parallel_for_test.cpp
        tests/engine/batch/simulation_result_test.cpp
        tests/engine/game/game_run_test.cpp
        tests/engine/game/resume_game_test.cpp
        tests/engine/game/scenario_validation_test.cpp
        tests/engine/policy/late_game_policy_test.cpp
        tests/engine/policy/policy_variants_test.cpp
        tests/engine/rules/apply_outcome_test.cpp
        tests/engine/sampling/outcome_sampler_test.cpp
        tests/integration/integration_test.cpp
        tests/integration/invariant_sweep_test.cpp
        tests/integration/policy_sensitivity_test.cpp
)
add_dependencies(gridiron_tests simulate)
target_compile_definitions(gridiron_tests PRIVATE
        SIMULATE_PATH="$<TARGET_FILE:simulate>"
)
target_link_libraries(simulate PRIVATE gridiron_cli gridiron_core gridiron_warnings)
target_link_libraries(gridiron_tests PRIVATE gtest_main gridiron_cli gridiron_core gridiron_warnings)
include(GoogleTest)
gtest_discover_tests(gridiron_tests)
