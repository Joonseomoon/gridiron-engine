#pragma once

#include "TeamProfile.h"

#include <cstdint>
#include <string>

namespace gridiron {

struct BatchMetadata {
    TeamProfile home;
    TeamProfile away;
    int games = 0;
    std::uint32_t batchSeed = 0;
    std::string commandLine;
};

} // namespace gridiron
