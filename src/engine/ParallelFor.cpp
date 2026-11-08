#include "ParallelFor.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <thread>
#include <vector>

namespace gridiron {
void parallelFor(int count, int threads, const std::function<void(int begin, int end)>& work) {
    if (threads < 1) {
        throw std::invalid_argument("threads must be positive");
    }

    if (count <= 0) {
        return;
    }

    const int workers = std::min(threads, count);

    if (workers == 1) {
        work(0, count);
        return;
    }

    std::vector<std::thread> pool;
    pool.reserve(static_cast<std::size_t>(workers));
    constexpr int kChunkSize = 64;
    const int chunks = (count - 1) / kChunkSize + 1;
    std::atomic<int> nextChunk{0};
    std::vector<std::exception_ptr> errors(static_cast<std::size_t>(chunks));

    const auto runWorker = [&]() {
        while (true) {
            const int chunk = nextChunk.fetch_add(1);
            if (chunk >= chunks) {
                return;
            }

            const int begin = chunk * kChunkSize;
            const int end = begin + std::min(kChunkSize, count - begin);

            try {
                work(begin, end);
            } catch (...) {
                errors[static_cast<std::size_t>(chunk)] = std::current_exception();
            }
        }
    };

    try {
        for (int worker = 0; worker < workers; ++worker) {
            pool.emplace_back(runWorker);
        }
    } catch (...) {
        for (auto& worker : pool) {
            worker.join();
        }
        throw;
    }

    for (auto& worker : pool) {
        worker.join();
    }

    for (const auto& error : errors) {
        if (error) {
            std::rethrow_exception(error);
        }
    }
}
} // namespace gridiron
