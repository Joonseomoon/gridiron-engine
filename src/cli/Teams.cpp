#include "Teams.h"
#include <cctype>

namespace gridiron {

namespace {
// name, off pass, off run, def pass, def run, turnover rate, FG rating
const std::vector<TeamProfile> teamProfiles = {
    {"BAL", 0.8, 0.8, 0.8, 0.7, 0.03, 0.94},   // balanced
    {"SEA", 0.7, 0.8, 0.9, 0.8, 0.02, 0.95},   // defense first
    {"LAR", 0.9, 0.8, 0.8, 0.9, 0.03, 0.92},   // super team
    {"KC", 0.5, 0.4, 0.8, 0.8, 0.02, 0.96},    // terrible offense
    {"HOU", 0.6, 0.4, 0.9, 0.85, 0.03, 0.94},  // defense only
    {"DAL", 0.95, 0.85, 0.4, 0.6, 0.02, 0.98}, // offense only
};
} // namespace

std::optional<TeamProfile> findTeam(std::string_view abbreviation) {
    std::string upper;
    upper.reserve(abbreviation.size());
    for (char c : abbreviation) {
        upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    for (const auto& teamProfile : teamProfiles) {
        if (teamProfile.name == upper) {
            return teamProfile;
        }
    }
    return std::nullopt;
}

std::vector<std::string> teamAbbreviations() {
    std::vector<std::string> teams;
    teams.reserve(teamProfiles.size());
    for (const auto& teamProfile : teamProfiles) {
        teams.push_back(teamProfile.name);
    }
    return teams;
}

} // namespace gridiron
