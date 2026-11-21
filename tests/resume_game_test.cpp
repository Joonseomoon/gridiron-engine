#include <gtest/gtest.h>
#include <optional>
#include <random>
#include <tuple>
#include <vector>

#include "../src/engine/CandidateAction.h"
#include "../src/engine/DefaultPolicy.h"
#include "../src/engine/GameRunner.h"
#include "../src/engine/ModelConstants.h"

namespace {
const gridiron::TeamProfile kBal{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
const gridiron::TeamProfile kSea{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
const gridiron::Matchup kMatchup{.home = kBal, .away = kSea};
const gridiron::DefaultPolicy kDefault;
const gridiron::Policies kDefaults{kDefault, kDefault};

// The M4.md example: BAL (home) trails SEA 24-27, 3:14 left, 4th and 2 at SEA's 43.
// The default policy goes for it here (43 yards out, 2 to go).
gridiron::GameState exampleScenario() {
    return gridiron::GameState{.quarter = 4,
                               .time_remaining_seconds = 194,
                               .possession = gridiron::Possession::Home,
                               .down = 4,
                               .distance = 2,
                               .ball_position = 57,
                               .home_score = 24,
                               .away_score = 27};
}

auto asTuple(const gridiron::GameState& s) {
    return std::tuple(s.quarter, s.time_remaining_seconds, s.home_timeouts, s.away_timeouts, s.opening_possession,
                      s.possession, s.down, s.distance, s.ball_position, s.home_score, s.away_score, s.game_over,
                      s.overtime_possessions_completed);
}

void expectFinished(const gridiron::GameState& state) {
    EXPECT_TRUE(state.game_over);
    EXPECT_TRUE(state.quarter == 4 || state.quarter == 5);
    if (state.quarter == 4) {
        EXPECT_EQ(state.time_remaining_seconds, 0);
        EXPECT_NE(state.home_score, state.away_score);
    }
}

// Records every play's type and the state before it.
struct PlayRecorder {
    std::vector<gridiron::PlayType> types;
    std::vector<gridiron::GameState> befores;

    gridiron::PlayObserver observer() {
        return [this](const gridiron::GameState& before, const gridiron::PlayOutcome& outcome,
                      const gridiron::PlayResult&) {
            types.push_back(outcome.play_type);
            befores.push_back(before);
        };
    }
};

// Always runs, so any play that is not a run must have been forced.
class AlwaysRunPolicy : public gridiron::Policy {
public:
    gridiron::PlayType choosePlay(const gridiron::GameState&, const gridiron::Matchup&,
                                  gridiron::RandomSource&) const override {
        return gridiron::PlayType::Run;
    }
    bool callTimeout(const gridiron::GameState&, const gridiron::PlayOutcome&, gridiron::Possession) const override {
        return false;
    }
};
} // namespace

// --- resumeGame -------------------------------------------------------------

TEST(ResumeGameTest, FinishesFromTheExampleScenario) {
    for (unsigned seed = 1; seed <= 200; ++seed) {
        std::mt19937 rng(seed);
        expectFinished(gridiron::resumeGame(exampleScenario(), kMatchup, kDefaults, std::nullopt, rng));
    }
}

TEST(ResumeGameTest, SameSeedGivesSameGame) {
    std::mt19937 rng1(42);
    std::mt19937 rng2(42);
    const auto first = gridiron::resumeGame(exampleScenario(), kMatchup, kDefaults, std::nullopt, rng1);
    const auto second = gridiron::resumeGame(exampleScenario(), kMatchup, kDefaults, std::nullopt, rng2);
    EXPECT_EQ(asTuple(first), asTuple(second));
    EXPECT_EQ(rng1, rng2);
}

// No coin toss: the first snap is exactly the supplied state, even with the away team on offense.
TEST(ResumeGameTest, FirstSnapIsTheSuppliedState) {
    gridiron::GameState start = exampleScenario();
    start.possession = gridiron::Possession::Away;
    start.opening_possession = gridiron::Possession::Away;
    for (unsigned seed = 1; seed <= 20; ++seed) {
        PlayRecorder recorder;
        std::mt19937 rng(seed);
        gridiron::resumeGame(start, kMatchup, kDefaults, std::nullopt, rng, recorder.observer());
        ASSERT_FALSE(recorder.befores.empty());
        EXPECT_EQ(asTuple(recorder.befores.front()), asTuple(start)) << "seed " << seed;
    }
}

// runGame is the coin toss followed by resumeGame from the opening kickoff.
TEST(ResumeGameTest, RunGameIsCoinTossPlusResume) {
    for (unsigned seed = 1; seed <= 50; ++seed) {
        std::mt19937 runRng(seed);
        const auto viaRunGame = gridiron::runGame(kMatchup, runRng, kDefaults, gridiron::PlayObserver{});

        std::mt19937 resumeRng(seed);
        gridiron::GameState start;
        start.ball_position = gridiron::kKickoffSpot;
        std::bernoulli_distribution coinToss(0.5);
        start.opening_possession = coinToss(resumeRng) ? gridiron::Possession::Home : gridiron::Possession::Away;
        start.possession = start.opening_possession;
        const auto viaResume = gridiron::resumeGame(start, kMatchup, kDefaults, std::nullopt, resumeRng);

        EXPECT_EQ(asTuple(viaRunGame), asTuple(viaResume)) << "seed " << seed;
    }
}

// Tied with one snap left: most games reach overtime, and every game still finishes.
TEST(ResumeGameTest, TiedLateInRegulationGoesToOvertime) {
    const gridiron::GameState start{.quarter = 4,
                                    .time_remaining_seconds = 1,
                                    .possession = gridiron::Possession::Home,
                                    .ball_position = 25,
                                    .home_score = 17,
                                    .away_score = 17};
    int overtimeGames = 0;
    for (unsigned seed = 1; seed <= 100; ++seed) {
        std::mt19937 rng(seed);
        const auto final = gridiron::resumeGame(start, kMatchup, kDefaults, std::nullopt, rng);
        expectFinished(final);
        if (final.quarter == 5) {
            ++overtimeGames;
        }
    }
    EXPECT_GT(overtimeGames, 90);
}

// --- firstAction ------------------------------------------------------------

TEST(ResumeGameTest, ForcedActionChoosesTheFirstPlay) {
    const std::vector<std::pair<gridiron::CandidateAction, gridiron::PlayType>> cases{
        {gridiron::CandidateAction::Run, gridiron::PlayType::Run},
        {gridiron::CandidateAction::Pass, gridiron::PlayType::Pass},
        {gridiron::CandidateAction::Punt, gridiron::PlayType::Punt},
        {gridiron::CandidateAction::FieldGoal, gridiron::PlayType::FieldGoalAttempt},
    };
    for (const auto& [action, expected] : cases) {
        for (unsigned seed = 1; seed <= 20; ++seed) {
            PlayRecorder recorder;
            std::mt19937 rng(seed);
            gridiron::resumeGame(exampleScenario(), kMatchup, kDefaults, action, rng, recorder.observer());
            ASSERT_FALSE(recorder.types.empty());
            EXPECT_EQ(recorder.types.front(), expected) << "seed " << seed;
        }
    }
}

// The policy only ever runs, so every play after the forced pass must be a run.
TEST(ResumeGameTest, ForcedActionAppliesToTheFirstPlayOnly) {
    const AlwaysRunPolicy alwaysRun;
    const gridiron::Policies policies{alwaysRun, alwaysRun};
    PlayRecorder recorder;
    std::mt19937 rng(7);
    gridiron::resumeGame(exampleScenario(), kMatchup, policies, gridiron::CandidateAction::Pass, rng,
                         recorder.observer());

    ASSERT_GT(recorder.types.size(), 1u);
    EXPECT_EQ(recorder.types.front(), gridiron::PlayType::Pass);
    for (std::size_t i = 1; i < recorder.types.size(); ++i) {
        EXPECT_EQ(recorder.types[i], gridiron::PlayType::Run) << "play " << i;
    }
}

// The default policy also goes for it at the example, with the same draw, so forcing GoForIt changes nothing.
TEST(ResumeGameTest, GoForItUsesTheDefaultRunPassDraw) {
    for (unsigned seed = 1; seed <= 50; ++seed) {
        std::mt19937 forcedRng(seed);
        std::mt19937 policyRng(seed);
        const auto forced =
            gridiron::resumeGame(exampleScenario(), kMatchup, kDefaults, gridiron::CandidateAction::GoForIt, forcedRng);
        const auto chosen = gridiron::resumeGame(exampleScenario(), kMatchup, kDefaults, std::nullopt, policyRng);
        EXPECT_EQ(asTuple(forced), asTuple(chosen)) << "seed " << seed;
    }
}

TEST(ResumeGameTest, GoForItProducesBothRunAndPass) {
    int runs = 0;
    constexpr int kGames = 1000;
    for (unsigned seed = 1; seed <= kGames; ++seed) {
        PlayRecorder recorder;
        std::mt19937 rng(seed);
        gridiron::resumeGame(exampleScenario(), kMatchup, kDefaults, gridiron::CandidateAction::GoForIt, rng,
                             recorder.observer());
        ASSERT_FALSE(recorder.types.empty());
        const gridiron::PlayType first = recorder.types.front();
        ASSERT_TRUE(first == gridiron::PlayType::Run || first == gridiron::PlayType::Pass);
        if (first == gridiron::PlayType::Run) {
            ++runs;
        }
    }
    EXPECT_NEAR(runs / static_cast<double>(kGames), 0.5, 0.05);
}
