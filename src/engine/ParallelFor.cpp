#include "ParallelFor.h"

#include <algorithm>
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
    std::vector<std::exception_ptr> errors(static_cast<std::size_t>(workers));

    const auto runBlock = [&work, &errors](int worker, int begin, int end) {
        try {
            work(begin, end);
        } catch (...) {
            errors[static_cast<std::size_t>(worker)] = std::current_exception();
        }
    };

    try {
        int begin = 0;

        for (int worker = 0; worker < workers; ++worker) {
            const int blockSize = count / workers + (worker < count % workers ? 1 : 0);
            const int end = begin + blockSize;

            pool.emplace_back(runBlock, worker, begin, end);
            begin = end;
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
