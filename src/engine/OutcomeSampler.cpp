#include "OutcomeSampler.h"
#include "DefaultPolicy.h"
#include "ModelConstants.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

namespace gridiron {

using NormalParams = std::normal_distribution<double>::param_type;

namespace {
// Bigger gains take longer to run and to reset from.
int samplePlayTime(const int yardsGained, RandomSource& random) {
    if (yardsGained > 20) {
        return std::max(15, static_cast<int>(std::round(random.normal(random.rng, NormalParams{30, 5}))));
    }
    return std::max(10, static_cast<int>(std::round(random.normal(random.rng, NormalParams{20, 5}))));
}

bool sampleOutOfBounds(RandomSource& random) {
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(random.rng) < kOutOfBoundsRate;
}
} // namespace

PlayOutcome sampleRunOutcome(const TeamProfile& offense, const TeamProfile& defense, RandomSource& random) {
    const int baseYards{2};
    const double expectedYards{baseYards + 10 * (offense.offensive_run_rating - defense.defensive_run_rating)};
    const int playYards = static_cast<int>(std::round(random.normal(random.rng, NormalParams{expectedYards, 3.0})));
    const int playTime = samplePlayTime(playYards, random);

    std::uniform_real_distribution<double> turnoverDistribution(0.0, 1.0);
    const bool turnover = turnoverDistribution(random.rng) < offense.turnover_rate;

    if (turnover) {
        return PlayOutcome{.play_type = PlayType::Run,
                           .outcome_type = OutcomeType::Turnover,
                           .yards_gained = playYards,
                           .elapsed_seconds = kSnapSeconds,
                           .runoff_seconds = playTime - kSnapSeconds};
    }
    return PlayOutcome{.play_type = PlayType::Run,
                       .outcome_type = OutcomeType::Normal,
                       .yards_gained = playYards,
                       .elapsed_seconds = kSnapSeconds,
                       .runoff_seconds = playTime - kSnapSeconds,
                       .out_of_bounds = sampleOutOfBounds(random)};
}

PlayOutcome samplePassOutcome(const TeamProfile& offense, const TeamProfile& defense, RandomSource& random) {
    const int baseYards{4};
    const double expectedYards{baseYards + 15 * (offense.offensive_pass_rating - defense.defensive_pass_rating)};

    std::uniform_real_distribution<double> turnoverDistribution(0.0, 1.0);
    const bool turnover = turnoverDistribution(random.rng) < offense.turnover_rate;

    if (!turnover) {
        std::uniform_real_distribution<double> incompleteDistribution(0.0, 1.0);
        if (incompleteDistribution(random.rng) < kIncompleteRate) {
            return PlayOutcome{.play_type = PlayType::Pass,
                               .outcome_type = OutcomeType::Incomplete,
                               .yards_gained = 0,
                               .elapsed_seconds = kSnapSeconds,
                               .runoff_seconds = 0};
        }
    }

    const int playYards = static_cast<int>(std::round(random.normal(random.rng, NormalParams{expectedYards, 5.0})));
    const int playTime = samplePlayTime(playYards, random);

    if (turnover) {
        return PlayOutcome{.play_type = PlayType::Pass,
                           .outcome_type = OutcomeType::Turnover,
                           .yards_gained = playYards,
                           .elapsed_seconds = kSnapSeconds,
                           .runoff_seconds = playTime - kSnapSeconds};
    }
    return PlayOutcome{.play_type = PlayType::Pass,
                       .outcome_type = OutcomeType::Normal,
                       .yards_gained = playYards,
                       .elapsed_seconds = kSnapSeconds,
                       .runoff_seconds = playTime - kSnapSeconds,
                       .out_of_bounds = sampleOutOfBounds(random)};
}

PlayOutcome sampleFieldGoalOutcome(const TeamProfile& kicker, int distanceToGoal, RandomSource& random) {
    const int kickDistance = distanceToGoal + 17;
    const double successProbability = std::clamp(kicker.field_goal_rating - 0.01 * (kickDistance - 20), 0.01, 0.99);

    std::uniform_real_distribution<double> fieldGoalDistribution(0.0, 1.0);
    const bool made = fieldGoalDistribution(random.rng) < successProbability;

    return PlayOutcome{.play_type = PlayType::FieldGoalAttempt,
                       .outcome_type = made ? OutcomeType::FieldGoalMade : OutcomeType::Turnover,
                       .elapsed_seconds = kSnapSeconds,
                       .runoff_seconds = samplePlayTime(0, random) - kSnapSeconds};
}

PlayOutcome samplePuntOutcome(RandomSource& random) {
    const int puntYards =
        static_cast<int>(std::round(random.normal(random.rng, NormalParams{kPuntNetMeanYards, kPuntNetSdYards})));

    return PlayOutcome{.play_type = PlayType::Punt,
                       .outcome_type = OutcomeType::Punted,
                       .yards_gained = puntYards,
                       .elapsed_seconds = kPuntSeconds};
}

PlayOutcome samplePlayOutcome(const GameState& gameState, const Matchup& matchup, RandomSource& random) {
    const DefaultPolicy policy;
    const PlayType playType = policy.choosePlay(gameState, matchup, random);
    return samplePlayOutcome(gameState, matchup, playType, random);
}

PlayOutcome samplePlayOutcome(const GameState& gameState, const Matchup& matchup, PlayType playType,
                              RandomSource& random) {
    const TeamProfile& offense = (gameState.possession == Possession::Home) ? matchup.home : matchup.away;
    const TeamProfile& defense = (gameState.possession == Possession::Home) ? matchup.away : matchup.home;

    switch (playType) {
    case PlayType::Pass: return samplePassOutcome(offense, defense, random);
    case PlayType::Run: return sampleRunOutcome(offense, defense, random);
    case PlayType::FieldGoalAttempt: {
        const int distanceToGoal = 100 - gameState.ball_position;
        return sampleFieldGoalOutcome(offense, distanceToGoal, random);
    }
    case PlayType::Punt: return samplePuntOutcome(random);
    }
    throw std::logic_error("unhandled PlayType in samplePlayOutcome");
}
} // namespace gridiron
