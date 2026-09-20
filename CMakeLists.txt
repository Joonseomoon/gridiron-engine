cmake_minimum_required(VERSION 3.20)
project(gridiron_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY  https://github.com/google/googletest
        GIT_TAG         v1.18.0
)

FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(gridiron_engine src/main.cpp)
add_executable(gridiron_tests 
                tests/apply_outcome_test.cpp
                tests/outcome_sampler_test.cpp
                src/engine/OutcomeSampler.cpp)
target_link_libraries(gridiron_tests PRIVATE gtest_main)
include(GoogleTest)
gtest_discover_tests(gridiron_tests)
