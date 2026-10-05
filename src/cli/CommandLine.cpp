#include "CommandLine.h"

#include <charconv>

namespace gridiron {

std::variant<Options, std::string> parseArgs(const std::vector<std::string>& args) {
    std::vector<std::string> teams;
    std::optional<std::uint32_t> seed;

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
        if (arg.starts_with("-")) {
            return "unknown option '" + arg + "'";
        }
        teams.push_back(arg);
    }

    if (teams.size() != 2) {
        return "expected two teams: simulate HOME AWAY [--seed N]";
    }

    return Options{teams[0], teams[1], seed};
}
} // namespace gridiron
