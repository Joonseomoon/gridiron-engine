// Developer tool: run one seeded game and dump every play as raw engine state.
// Not the M1.3 CLI — no team selection, fixed BAL vs SEA profiles, and output
// meant for inspecting the engine, not for fans.
//
// Usage: ./build/gridiron_debug_game [seed]   (default seed 1000)

#include <cstdio>
#include <cstdlib>
#include <random>

#include "../src/engine/GameRunner.h"

namespace {

const char* toString(gridiron::PlayType type) {
    switch (type) {
    case gridiron::PlayType::Run: return "Run";
    case gridiron::PlayType::Pass: return "Pass";
    case gridiron::PlayType::FieldGoalAttempt: return "FG";
    }
    return "?";
}

const char* toString(gridiron::OutcomeType type) {
    switch (type) {
    case gridiron::OutcomeType::Normal: return "Normal";
    case gridiron::OutcomeType::Turnover: return "Turnover";
    case gridiron::OutcomeType::FieldGoalMade: return "FG made";
    }
    return "?";
}

const char* teamWithBall(const gridiron::GameState& s) {
    return (s.possession == gridiron::Possession::Home ? s.home : s.away).name.c_str();
}

} // namespace

int main(int argc, char** argv) {
    const unsigned seed = argc > 1 ? static_cast<unsigned>(std::strtoul(argv[1], nullptr, 10)) : 1000;

    const gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.95};
    const gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.95};
    std::mt19937 rng(seed);

    std::printf("%-3s %-6s %-4s %-9s %-4s | %-5s %-9s %5s %4s | %s\n", "Q", "Clock", "Ball", "Down&Dist", "Spot",
                "Play", "Outcome", "Yards", "Secs", "Score after");
    int plays = 0;
    const auto onPlay = [&](const gridiron::GameState& before, const gridiron::PlayOutcome& outcome,
                            const gridiron::GameState& after) {
        ++plays;
        char downDistance[16];
        std::snprintf(downDistance, sizeof downDistance, "%d & %d", before.down, before.distance);
        std::printf("Q%-2d %2d:%02d  %-4s %-9s %-4d | %-5s %-9s %5d %4d | %s %d - %s %d\n", before.quarter,
                    before.time_remaining_seconds / 60, before.time_remaining_seconds % 60, teamWithBall(before),
                    downDistance, before.ball_position, toString(outcome.play_type), toString(outcome.outcome_type),
                    outcome.yards_gained, outcome.elapsed_seconds, after.home.name.c_str(), after.home_score,
                    after.away.name.c_str(), after.away_score);
    };

    const gridiron::GameState final = gridiron::runGame(home, away, rng, onPlay);

    std::printf("\nOpening possession: %s\n",
                (final.opening_possession == gridiron::Possession::Home ? home : away).name.c_str());
    std::printf("Final: %s %d - %s %d (%d plays)\n", home.name.c_str(), final.home_score, away.name.c_str(),
                final.away_score, plays);
    const char* winner = final.home_score > final.away_score   ? home.name.c_str()
                         : final.away_score > final.home_score ? away.name.c_str()
                                                               : "Tie";
    std::printf("Winner: %s\nSeed: %u\n", winner, seed);
    return 0;
}
