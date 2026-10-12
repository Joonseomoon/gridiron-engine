#include "CommandLine.h"

#include "Teams.h"

#include <charconv>

namespace gridiron {

std::variant<Options, std::string> parseArgs(const std::vector<std::string>& args) {
    std::vector<std::string> teams;
    std::optional<std::uint32_t> seed;
    std::optional<int> games;

    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "--seed") {
            if (seed) {
                return "--seed given more than once";
            }
            if (i + 1 >= args.size()) {
                return "--seed needs a value";
            }
            const std::string& text = args[i + 1];
            std::uint32_t value = 0;
            const char* end = text.data() + text.size();
            const auto [stoppedAt, error] = std::from_chars(text.data(), end, value);
            if (error != std::errc{} || stoppedAt != end) {
                return "invalid seed '" + text + "'";
            }
            seed = value;
            ++i;
            continue;
        }
        if (arg == "--games") {
            if (games) {
                return "--games given more than once";
            }
            if (i + 1 >= args.size()) {
                return "--games needs a value";
            }
            const std::string& text = args[i + 1];
            int value = 0;
            const char* end = text.data() + text.size();
            const auto [stoppedAt, error] = std::from_chars(text.data(), end, value);
            if (error != std::errc{} || stoppedAt != end) {
                return "invalid game count '" + text + "'";
            }
            if (value > kMaxGames || value < kMinGames) {
                return "--games must be between " + std::to_string(kMinGames) + " and " + std::to_string(kMaxGames);
            }

            games = value;
            ++i;
            continue;
        }
        if (arg.starts_with("-")) {
            return "unknown option '" + arg + "'";
        }
        teams.push_back(arg);
    }

    if (teams.size() != 2) {
        return "expected two teams";
    }

    return Options{teams[0], teams[1], seed, games};
}

namespace {
std::string availableTeams() {
    std::string list;
    for (const auto& name : teamAbbreviations()) {
        if (!list.empty()) {
            list += ", ";
        }
        list += name;
    }
    return list;
}
} // namespace

std::variant<Matchup, std::string> resolveTeams(const Options& options) {
    const auto home = findTeam(options.home);
    if (!home) {
        return "unknown team '" + options.home + "'. Available: " + availableTeams();
    }

    const auto away = findTeam(options.away);
    if (!away) {
        return "unknown team '" + options.away + "'. Available: " + availableTeams();
    }

    if (home->name == away->name) {
        return "home and away must be different teams";
    }

    return Matchup{*home, *away};
}
} // namespace gridiron
