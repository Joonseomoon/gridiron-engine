#include <gtest/gtest.h>

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
    const std::vector<std::pair<int, int>> cases = {{8, 4}, {10, 3},  {7, 4},    {3, 8},
                                                    {1, 4}, {100, 1}, {1000, 4}, {1001, 3}};

    for (const auto& [count, threads] : cases) {
        SCOPED_TRACE("count " + std::to_string(count) + ", threads " + std::to_string(threads));
        std::vector<std::atomic<int>> hits(static_cast<std::size_t>(count));

        gridiron::parallelFor(count, threads, [&](int begin, int end) {
            for (int i = begin; i < end; ++i) {
                ++hits[static_cast<std::size_t>(i)];
            }
        });

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
        const int count = 1000;

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

TEST(ParallelForTest, SeveralFailuresGiveTheExceptionFromTheEarliestBlock) {
    // Repeated because which block fails first in real time varies from run to run.
    for (int attempt = 0; attempt < 50; ++attempt) {
        SCOPED_TRACE(attempt);

        try {
            gridiron::parallelFor(1000, 4,
                                  [](int begin, int) { throw WorkerError("block at " + std::to_string(begin)); });
            FAIL() << "expected a WorkerError";
        } catch (const WorkerError& error) {
            EXPECT_STREQ(error.what(), "block at 0");
        }
    }
}

TEST(ParallelForTest, OtherBlocksStillRunWhenOneFails) {
    const int count = 1000;
    std::atomic<int> failedBlockSize{0};
    std::atomic<int> finished{0};

    EXPECT_THROW(gridiron::parallelFor(count, 4,
                                       [&](int begin, int end) {
                                           if (begin == 0) {
                                               failedBlockSize = end;
                                               throw WorkerError("first block failed");
                                           }
                                           finished += end - begin;
                                       }),
                 WorkerError);

    EXPECT_EQ(finished.load(), count - failedBlockSize.load());
}
