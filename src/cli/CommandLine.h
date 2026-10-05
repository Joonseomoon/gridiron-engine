#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace gridiron {
struct Options {
    std::string home;
    std::string away;
    std::optional<std::uint32_t> seed;
};

std::variant<Options, std::string> parseArgs(const std::vector<std::string>& args);

} // namespace gridiron
