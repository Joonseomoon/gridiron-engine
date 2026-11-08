#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../src/engine/ParallelFor.h"

namespace {
// A type of its own, so a test catching it proves the original exception type survived the trip between threads.
struct WorkerError : std::runtime_error {
    using std::runtime_error::runtime_error;
};
} // namespace

TEST(ParallelForTest, RunsEveryIndexExactlyOnce) {
    const std::vector<std::pair<int, int>> cases = {{8, 4}, {10, 3}, {7, 4}, {3, 8}, {1, 4}, {100, 1}};

    for (const auto& [count, threads] : cases) {
        SCOPED_TRACE("count " + std::to_string(count) + ", threads " + std::to_string(threads));
        std::vector<std::atomic<int>> hits(static_cast<std::size_t>(count));
        std::atomic<int> blocks{0};

        gridiron::parallelFor(count, threads, [&](int begin, int end) {
            ++blocks;
            for (int i = begin; i < end; ++i) {
                ++hits[static_cast<std::size_t>(i)];
            }
        });

        EXPECT_EQ(blocks.load(), std::min(count, threads));
        for (const auto& hit : hits) {
            EXPECT_EQ(hit.load(), 1);
        }
    }
}

TEST(ParallelForTest, NothingToDoNeverCallsWork) {
    for (const int count : {0, -1}) {
        SCOPED_TRACE(count);
        std::atomic<int> calls{0};
        gridiron::parallelFor(count, 4, [&](int, int) { ++calls; });
        EXPECT_EQ(calls.load(), 0);
    }
}

TEST(ParallelForTest, RejectsThreadCountBelowOneEvenWithNothingToDo) {
    const auto work = [](int, int) {};
    EXPECT_THROW(gridiron::parallelFor(10, 0, work), std::invalid_argument);
    EXPECT_THROW(gridiron::parallelFor(0, 0, work), std::invalid_argument);
}

TEST(ParallelForTest, WorkerExceptionReachesCallerWithTypeAndMessage) {
    for (const int threads : {1, 2, 4}) {
        SCOPED_TRACE(threads);
        const int count = 8;

        try {
            gridiron::parallelFor(count, threads, [&](int, int end) {
                if (end == count) {
                    throw WorkerError("last block failed");
                }
            });
            FAIL() << "expected a WorkerError";
        } catch (const WorkerError& error) {
            EXPECT_STREQ(error.what(), "last block failed");
        }
    }
}

TEST(ParallelForTest, SeveralFailingWorkersGiveTheLowestNumberedWorkersException) {
    // Repeated because which worker fails first in real time varies from run to run.
    for (int attempt = 0; attempt < 50; ++attempt) {
        SCOPED_TRACE(attempt);

        try {
            gridiron::parallelFor(8, 4, [](int begin, int) { throw WorkerError("block at " + std::to_string(begin)); });
            FAIL() << "expected a WorkerError";
        } catch (const WorkerError& error) {
            EXPECT_STREQ(error.what(), "block at 0");
        }
    }
}

TEST(ParallelForTest, OtherWorkersFinishWhenOneFails) {
    std::atomic<int> finished{0};

    EXPECT_THROW(gridiron::parallelFor(8, 4,
                                       [&](int begin, int) {
                                           if (begin == 0) {
                                               throw WorkerError("first block failed");
                                           }
                                           ++finished;
                                       }),
                 WorkerError);

    EXPECT_EQ(finished.load(), 3);
}
