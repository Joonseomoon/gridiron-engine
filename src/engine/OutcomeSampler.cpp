#include "OutcomeSampler.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

namespace gridiron {

using NormalParams = std::normal_distribution<double>::param_type;

// Bigger gains take longer to run and to reset from.
int samplePlayTime(const int yardsGained, RandomSource& random) {
    if (yardsGained > 20) {
        return std::max(15, static_cast<int>(std::round(random.normal(random.rng, NormalParams{30, 5}))));
    }
    return std::max(10, static_cast<int>(std::round(random.normal(random.rng, NormalParams{20, 5}))));
}

PlayType choosePlayType(const GameState& gameState, RandomSource& random) {
    const int distanceToGoal = 100 - gameState.ball_position;
    if (gameState.down == 4 && distanceToGoal <= 40) {
        return PlayType::FieldGoalAttempt;
    }
    std::uniform_real_distribution<double> passOrRunDistribution(0.0, 1.0);
    if (passOrRunDistribution(random.rng) > 0.5) {
        return PlayType::Run;
    }
    return PlayType::Pass;
}

PlayOutcome sampleRunOutcome(const TeamProfile& offense, const TeamProfile& defense, RandomSource& random) {
    const int baseYards{2};
    const double expectedYards{baseYards + 10 * (offense.offensive_run_rating - defense.defensive_run_rating)};
    const int playYards = static_cast<int>(std::round(random.normal(random.rng, NormalParams{expectedYards, 3.0})));
    const int playTime = samplePlayTime(playYards, random);

    std::uniform_real_distribution<double> turnoverDistribution(0.0, 1.0);
    bool turnover = turnoverDistribution(random.rng) < offense.turnover_rate;

    if (turnover) {
        return PlayOutcome{PlayType::Run, OutcomeType::Turnover, playYards, playTime};
    }
    return PlayOutcome{PlayType::Run, OutcomeType::Normal, playYards, playTime};
}

PlayOutcome samplePassOutcome(const TeamProfile& offense, const TeamProfile& defense, RandomSource& random) {
    const int baseYards{4};
    const double expectedYards{baseYards + 15 * (offense.offensive_pass_rating - defense.defensive_pass_rating)};
    const int playYards = static_cast<int>(std::round(random.normal(random.rng, NormalParams{expectedYards, 5.0})));
    const int playTime = samplePlayTime(playYards, random);

    std::uniform_real_distribution<double> turnoverDistribution(0.0, 1.0);
    bool turnover = turnoverDistribution(random.rng) < offense.turnover_rate;

    if (turnover) {
        return PlayOutcome{PlayType::Pass, OutcomeType::Turnover, playYards, playTime};
    }
    return PlayOutcome{PlayType::Pass, OutcomeType::Normal, playYards, playTime};
}

PlayOutcome sampleFieldGoalOutcome(const TeamProfile& kicker, int distanceToGoal, RandomSource& random) {
    const int kickDistance = distanceToGoal + 17;
    const double successProbability = std::clamp(kicker.field_goal_rating - 0.01 * (kickDistance - 20), 0.01, 0.99);

    std::uniform_real_distribution<double> fieldGoalDistribution(0.0, 1.0);
    const bool made = fieldGoalDistribution(random.rng) < successProbability;

    return PlayOutcome{.play_type = PlayType::FieldGoalAttempt,
                       .outcome_type = made ? OutcomeType::FieldGoalMade : OutcomeType::Turnover,
                       .elapsed_seconds = samplePlayTime(0, random)};
}

PlayOutcome samplePlayOutcome(const GameState& gameState, RandomSource& random) {
    PlayType playType = choosePlayType(gameState, random);
    const TeamProfile& offense = (gameState.possession == Possession::Home) ? gameState.home : gameState.away;
    const TeamProfile& defense = (gameState.possession == Possession::Home) ? gameState.away : gameState.home;

    switch (playType) {
    case PlayType::Pass: return samplePassOutcome(offense, defense, random);
    case PlayType::Run: return sampleRunOutcome(offense, defense, random);
    case PlayType::FieldGoalAttempt: {
        const int distanceToGoal = 100 - gameState.ball_position;
        return sampleFieldGoalOutcome(offense, distanceToGoal, random);
    }
    }
    throw std::logic_error("unhandled PlayType in samplePlayOutcome");
}
} // namespace gridiron
