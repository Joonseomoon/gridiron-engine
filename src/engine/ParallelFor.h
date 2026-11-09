#pragma once

#include <functional>

namespace gridiron {
// Splits 0..count into blocks that up to `threads` threads claim as they finish, calling work(begin, end) for each.
void parallelFor(int count, int threads, const std::function<void(int begin, int end)>& work);
} // namespace gridiron
