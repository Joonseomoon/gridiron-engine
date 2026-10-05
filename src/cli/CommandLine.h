#pragma once

#include "../engine/TeamProfile.h"

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

struct Matchup {
    TeamProfile home;
    TeamProfile away;
};

// parses simulate BAL SEA --seed 1000 into Options or error string message
std::variant<Options, std::string> parseArgs(const std::vector<std::string>& args);

// Looks up both teams from parsed Options. Returns the matchup, or an error message
std::variant<Matchup, std::string> resolveTeams(const Options& options);

} // namespace gridiron
