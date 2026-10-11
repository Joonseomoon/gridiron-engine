#include "OutcomeSampler.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

namespace gridiron {

// Bigger gains take longer to run and to reset from.
int samplePlayTime(const int yardsGained, std::mt19937& rng) {
    if (yardsGained > 20) {
        std::normal_distribution<double> largePlayTimeDistribution(30, 5);
        return std::max(15, static_cast<int>(std::round(largePlayTimeDistribution(rng))));
    }
    std::normal_distribution<double> smallPlayTimeDistribution(20, 5);
    return std::max(10, static_cast<int>(std::round(smallPlayTimeDistribution(rng))));
}

PlayType choosePlayType(const GameState& gameState, std::mt19937& rng) {
    const int distanceToGoal = 100 - gameState.ball_position;
    if (gameState.down == 4 && distanceToGoal <= 40) {
        return PlayType::FieldGoalAttempt;
    }
    std::uniform_real_distribution<double> passOrRunDistribution(0.0, 1.0);
    if (passOrRunDistribution(rng) > 0.5) {
        return PlayType::Run;
    }
    return PlayType::Pass;
}

PlayOutcome sampleRunOutcome(const TeamProfile& offense, const TeamProfile& defense, std::mt19937& rng) {
    const int baseYards{2};
    const double expectedYards{baseYards + 10 * (offense.offensive_run_rating - defense.defensive_run_rating)};
    std::normal_distribution<double> playYardsDistribution(expectedYards, 3.0);
    const int playYards = static_cast<int>(std::round(playYardsDistribution(rng)));
    const int playTime = samplePlayTime(playYards, rng);

    std::uniform_real_distribution<double> turnoverDistribution(0.0, 1.0);
    bool turnover = turnoverDistribution(rng) < offense.turnover_rate;

    if (turnover) {
        return PlayOutcome{PlayType::Run, OutcomeType::Turnover, playYards, playTime};
    }
    return PlayOutcome{PlayType::Run, OutcomeType::Normal, playYards, playTime};
}

PlayOutcome samplePassOutcome(const TeamProfile& offense, const TeamProfile& defense, std::mt19937& rng) {
    const int baseYards{4};
    const double expectedYards{baseYards + 15 * (offense.offensive_pass_rating - defense.defensive_pass_rating)};
    std::normal_distribution<double> playYardsDistribution(expectedYards, 5.0);
    const int playYards = static_cast<int>(std::round(playYardsDistribution(rng)));
    const int playTime = samplePlayTime(playYards, rng);

    std::uniform_real_distribution<double> turnoverDistribution(0.0, 1.0);
    bool turnover = turnoverDistribution(rng) < offense.turnover_rate;

    if (turnover) {
        return PlayOutcome{PlayType::Pass, OutcomeType::Turnover, playYards, playTime};
    }
    return PlayOutcome{PlayType::Pass, OutcomeType::Normal, playYards, playTime};
}

PlayOutcome sampleFieldGoalOutcome(const TeamProfile& kicker, int distanceToGoal, std::mt19937& rng) {
    const int kickDistance = distanceToGoal + 17;
    const double successProbability = std::clamp(kicker.field_goal_rating - 0.01 * (kickDistance - 20), 0.01, 0.99);

    std::uniform_real_distribution<double> fieldGoalDistribution(0.0, 1.0);
    const bool made = fieldGoalDistribution(rng) < successProbability;

    return PlayOutcome{.play_type = PlayType::FieldGoalAttempt,
                       .outcome_type = made ? OutcomeType::FieldGoalMade : OutcomeType::Turnover,
                       .elapsed_seconds = samplePlayTime(0, rng)};
}

PlayOutcome samplePlayOutcome(const GameState& gameState, std::mt19937& rng) {
    PlayType playType = choosePlayType(gameState, rng);
    const TeamProfile& offense = (gameState.possession == Possession::Home) ? gameState.home : gameState.away;
    const TeamProfile& defense = (gameState.possession == Possession::Home) ? gameState.away : gameState.home;

    switch (playType) {
    case PlayType::Pass: return samplePassOutcome(offense, defense, rng);
    case PlayType::Run: return sampleRunOutcome(offense, defense, rng);
    case PlayType::FieldGoalAttempt: {
        const int distanceToGoal = 100 - gameState.ball_position;
        return sampleFieldGoalOutcome(offense, distanceToGoal, rng);
    }
    }
    throw std::logic_error("unhandled PlayType in samplePlayOutcome");
}
} // namespace gridiron
