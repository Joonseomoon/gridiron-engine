#pragma once

#include <functional>

namespace gridiron {
// splits 0..count across up to `threads` threads and calls work(begin, end) once per block
void parallelFor(int count, int threads, const std::function<void(int begin, int end)>& work);
} // namespace gridiron
