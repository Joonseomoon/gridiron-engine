#include "engine/game/GameRunner.h"

#include <optional>
#include <stdexcept>

#include "engine/core/ModelConstants.h"
#include "engine/core/PlayResult.h"
#include "engine/core/RandomSource.h"
#include "engine/policy/DefaultPolicy.h"
#include "engine/rules/ApplyOutcome.h"
#include "engine/rules/Overtime.h"
#include "engine/rules/Timeout.h"
#include "engine/sampling/OutcomeSampler.h"

namespace gridiron {

namespace {
PlayType forcedPlayType(CandidateAction action, RandomSource& random) {
    switch (action) {
    case CandidateAction::GoForIt: return chooseRunOrPass(random);
    case CandidateAction::Run: return PlayType::Run;
    case CandidateAction::Pass: return PlayType::Pass;
    case CandidateAction::FieldGoal: return PlayType::FieldGoalAttempt;
    case CandidateAction::Punt: return PlayType::Punt;
    }
    throw std::logic_error("unhandled CandidateAction");
}
} // namespace

GameState runGame(const Matchup& matchup, std::mt19937& rng) {
    return runGame(matchup, rng, PlayObserver{});
}

GameState runGame(const Matchup& matchup, std::mt19937& rng, const PlayObserver& onPlay) {
    const DefaultPolicy policy;
    const Policies policies{policy, policy};
    return runGame(matchup, rng, policies, onPlay);
}

GameState runGame(const Matchup& matchup, std::mt19937& rng, const Policies& policies, const PlayObserver& onPlay) {
    GameState state;
    state.ball_position = kKickoffSpot;

    std::bernoulli_distribution coinToss(0.5);
    state.opening_possession = coinToss(rng) ? Possession::Home : Possession::Away;
    state.possession = state.opening_possession;
    return resumeGame(state, matchup, policies, std::nullopt, rng, onPlay);
}

GameState resumeGame(GameState state, const Matchup& matchup, const Policies& policies,
                     std::optional<CandidateAction> firstAction, std::mt19937& rng, const PlayObserver& onPlay) {
    // One random source per game, so no stored normal value carries over from another game.
    RandomSource random{rng};

    // Bound four regulation quarters plus one overtime period by the minimum snap time.
    constexpr int maximumPlays = (900 / kSnapSeconds) * 4 + kOvertimeSeconds / kSnapSeconds;
    int plays = 0;
    while (!state.game_over) {
        if (plays >= maximumPlays) {
            throw std::runtime_error("Game exceeded maximum play count");
        }
        const Policy& policy = state.possession == Possession::Home ? policies.home : policies.away;
        PlayType playType;
        if (firstAction.has_value()) {
            playType = forcedPlayType(*firstAction, random);
            firstAction.reset();
        } else {
            playType = policy.choosePlay(state, matchup, random);
        }
        PlayOutcome outcome = samplePlayOutcome(state, matchup, playType, random);
        std::optional<GameState> before;
        if (onPlay || state.quarter == 5) {
            before = state;
        }

        const auto timeout = applyTimeout(state, outcome, policies);
        PlaySummary summary = applyOutcomeInPlace(state, outcome);

        if (before.has_value() && before->quarter == 5) {
            finishOvertimePlay(*before, state, summary);
        }

        if (state.game_over && state.quarter == 4 && state.home_score == state.away_score) {
            startOvertime(state, random);
            summary.period = PeriodEvent::OvertimeStart;
        }

        if (onPlay) {
            const PlayResult result{state, summary.event, summary.turnover, summary.period, timeout};
            onPlay(*before, outcome, result);
        }
        ++plays;
    }
    return state;
}

} // namespace gridiron
