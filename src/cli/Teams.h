#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "../engine/TeamProfile.h"

namespace gridiron {

// Look up a built-in team by abbreviation ("BAL"), empty if unknown.
std::optional<TeamProfile> findTeam(std::string_view abbreviation);

// Every built-in abbreviation, for error messages like "Available: BAL, SEA, ..."
std::vector<std::string> teamAbbreviations();
} // namespace gridiron
