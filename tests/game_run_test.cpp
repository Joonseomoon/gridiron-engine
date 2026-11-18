#include <gtest/gtest.h>
#include <random>
#include <tuple>

#include "../src/engine/ApplyOutcome.h"
#include "../src/engine/DefaultPolicy.h"
#include "../src/engine/GameRunner.h"
#include "../src/engine/Overtime.h"
#include "../src/engine/Timeout.h"

TEST(GameRunTest, GameOverTrue) {
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    gridiron::Matchup matchup{.home = home, .away = away};
    std::mt19937 rng(2000);

    gridiron::GameState playGame = gridiron::runGame(matchup, rng);

    EXPECT_TRUE(playGame.game_over);
    EXPECT_EQ(playGame.quarter, 4);
    EXPECT_EQ(playGame.time_remaining_seconds, 0);
}

TEST(GameRunTest, SameSeedFinalState) {
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    gridiron::Matchup matchup{.home = home, .away = away};
    std::mt19937 rng1(2000);
    std::mt19937 rng2(2000);

    gridiron::GameState playGame1 = gridiron::runGame(matchup, rng1);
    gridiron::GameState playGame2 = gridiron::runGame(matchup, rng2);

    EXPECT_EQ(playGame1.home_score, playGame2.home_score);
    EXPECT_EQ(playGame1.away_score, playGame2.away_score);
    EXPECT_EQ(playGame1.opening_possession, playGame2.opening_possession);
    EXPECT_EQ(playGame1.possession, playGame2.possession);
    EXPECT_EQ(playGame1.game_over, playGame2.game_over);
    EXPECT_EQ(playGame1.down, playGame2.down);
    EXPECT_EQ(playGame1.distance, playGame2.distance);
    EXPECT_EQ(playGame1.ball_position, playGame2.ball_position);
    EXPECT_EQ(playGame1.quarter, playGame2.quarter);
    EXPECT_EQ(playGame1.time_remaining_seconds, playGame2.time_remaining_seconds);
    EXPECT_EQ(rng1, rng2);
}

TEST(GameRunTest, DifferentSeedFinalState) {
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    gridiron::Matchup matchup{.home = home, .away = away};
    std::mt19937 rng1(2000);
    std::mt19937 rng2(4891);

    gridiron::GameState playGame1 = gridiron::runGame(matchup, rng1);
    gridiron::GameState playGame2 = gridiron::runGame(matchup, rng2);

    // Different seeds give different games. Any single field can match by chance (e.g. both end 1st & 10),
    // so compare the final state as a whole.
    const auto summary = [](const gridiron::GameState& s) {
        return std::tuple{s.home_score, s.away_score, s.possession, s.down, s.distance, s.ball_position};
    };
    EXPECT_NE(summary(playGame1), summary(playGame2));
    EXPECT_EQ(playGame1.quarter, playGame2.quarter);
    EXPECT_EQ(playGame1.time_remaining_seconds, playGame2.time_remaining_seconds);
    EXPECT_NE(rng1, rng2);
}

TEST(GameRunTest, RepeatAfterUnrelated) {
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    gridiron::Matchup matchup{.home = home, .away = away};
    std::mt19937 rng1(2000);
    std::mt19937 rng2(4891);
    std::mt19937 rng3(2000);

    gridiron::GameState playGame1 = gridiron::runGame(matchup, rng1);
    gridiron::runGame(matchup, rng2);
    gridiron::GameState playGame3 = gridiron::runGame(matchup, rng3);

    EXPECT_EQ(playGame1.home_score, playGame3.home_score);
    EXPECT_EQ(playGame1.away_score, playGame3.away_score);
    EXPECT_EQ(playGame1.opening_possession, playGame3.opening_possession);
    EXPECT_EQ(playGame1.possession, playGame3.possession);
    EXPECT_EQ(playGame1.game_over, playGame3.game_over);
    EXPECT_EQ(playGame1.down, playGame3.down);
    EXPECT_EQ(playGame1.distance, playGame3.distance);
    EXPECT_EQ(playGame1.ball_position, playGame3.ball_position);
    EXPECT_EQ(playGame1.quarter, playGame3.quarter);
    EXPECT_EQ(playGame1.time_remaining_seconds, playGame3.time_remaining_seconds);
    EXPECT_EQ(rng1, rng3);
}

TEST(GameRunTest, MultiSeedTermination) {
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    gridiron::Matchup matchup{.home = home, .away = away};
    for (unsigned seed : {2000u, 3500u, 5420u, 1290u, 9832u}) {
        SCOPED_TRACE(seed);
        std::mt19937 rng(seed);
        const auto state = gridiron::runGame(matchup, rng);
        EXPECT_TRUE(state.game_over);
        EXPECT_EQ(state.quarter, 4);
        EXPECT_EQ(state.time_remaining_seconds, 0);
        EXPECT_GE(state.down, 1);
        EXPECT_LE(state.down, 4);
        EXPECT_GE(state.ball_position, 1);
        EXPECT_LE(state.ball_position, 99);
        EXPECT_GT(state.distance, 0);
        EXPECT_GE(state.home_score, 0);
        EXPECT_GE(state.away_score, 0);
        EXPECT_LE(state.distance, 100 - state.ball_position);
    }
}

TEST(GameRunTest, OpeningCoinToss) {
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    gridiron::Matchup matchup{.home = home, .away = away};
    bool sawHome = false;
    bool sawAway = false;
    for (unsigned i = 0; i < 50; ++i) {

        std::mt19937 rng(i);
        gridiron::GameState playGame = gridiron::runGame(matchup, rng);
        if (playGame.opening_possession == gridiron::Possession::Home) {
            sawHome = true;
        } else if (playGame.opening_possession == gridiron::Possession::Away) {
            sawAway = true;
        }
    }
    EXPECT_TRUE(sawHome);
    EXPECT_TRUE(sawAway);
}

TEST(GameRunTest, AdvancesCallerRng) {
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    gridiron::Matchup matchup{.home = home, .away = away};
    std::mt19937 rng(2000);

    gridiron::runGame(matchup, rng);

    std::mt19937 newRng(2000);

    EXPECT_NE(rng, newRng);
}

TEST(GameRunTest, ExtremeProfilesStillTerminate) {
    // Best home possible team vs worst away possible team
    gridiron::TeamProfile homeBest{"BAL", 1.0, 1.0, 1.0, 1.0, 0.0, 1.0};
    gridiron::TeamProfile awayWorst{"SEA", 0.0, 0.0, 0.0, 0.0, 1.0, 0.0};
    gridiron::Matchup matchup1{.home = homeBest, .away = awayWorst};
    std::mt19937 rng1(2000);

    gridiron::GameState playGameBestWorst = gridiron::runGame(matchup1, rng1);

    EXPECT_TRUE(playGameBestWorst.game_over);
    EXPECT_EQ(playGameBestWorst.quarter, 4);
    EXPECT_EQ(playGameBestWorst.time_remaining_seconds, 0);

    // Best away possible team vs worst home possible team
    gridiron::TeamProfile homeWorst{"SEA", 0.0, 0.0, 0.0, 0.0, 1.0, 0.0};
    gridiron::TeamProfile awayBest{"BAL", 1.0, 1.0, 1.0, 1.0, 0.0, 1.0};
    gridiron::Matchup matchup2{.home = homeWorst, .away = awayBest};
    std::mt19937 rng2(2000);

    gridiron::GameState playGameWorstBest = gridiron::runGame(matchup2, rng2);

    EXPECT_TRUE(playGameWorstBest.game_over);
    EXPECT_EQ(playGameWorstBest.quarter, 4);
    EXPECT_EQ(playGameWorstBest.time_remaining_seconds, 0);

    // Turnover rates at highest
    gridiron::TeamProfile homeTurnover{"BAL", 0.8, 0.7, 0.8, 0.7, 1.0, 0.8};
    gridiron::TeamProfile awayTurnover{"SEA", 0.7, 0.9, 0.9, 0.5, 1.0, 0.8};
    gridiron::Matchup matchup3{.home = homeTurnover, .away = awayTurnover};
    std::mt19937 rng3(2000);

    gridiron::GameState playGameTurnover = gridiron::runGame(matchup3, rng3);

    EXPECT_TRUE(playGameTurnover.game_over);
    EXPECT_TRUE(playGameTurnover.quarter == 4 || playGameTurnover.quarter == 5);
    EXPECT_EQ(playGameTurnover.time_remaining_seconds, 0);

    // Turnover rates at lowest
    gridiron::TeamProfile homeNoTurnover{"BAL", 0.8, 0.7, 0.8, 0.7, 0.0, 0.8};
    gridiron::TeamProfile awayNoTurnover{"SEA", 0.7, 0.9, 0.9, 0.5, 0.0, 0.8};
    gridiron::Matchup matchup4{.home = homeNoTurnover, .away = awayNoTurnover};
    std::mt19937 rng4(2000);

    gridiron::GameState playGameNoTurnover = gridiron::runGame(matchup4, rng4);

    EXPECT_TRUE(playGameNoTurnover.game_over);
    EXPECT_EQ(playGameNoTurnover.quarter, 4);
    EXPECT_EQ(playGameNoTurnover.time_remaining_seconds, 0);
}

TEST(GameRunTest, UsesPolicyOfTeamWithPossession) {
    class AlwaysRunPolicy : public gridiron::DefaultPolicy {
    public:
        gridiron::PlayType choosePlay(const gridiron::GameState&, const gridiron::Matchup&,
                                      gridiron::RandomSource&) const override {
            return gridiron::PlayType::Run;
        }
    };

    class AlwaysPassPolicy : public gridiron::DefaultPolicy {
    public:
        gridiron::PlayType choosePlay(const gridiron::GameState&, const gridiron::Matchup&,
                                      gridiron::RandomSource&) const override {
            return gridiron::PlayType::Pass;
        }
    };

    const AlwaysRunPolicy homePolicy;
    const AlwaysPassPolicy awayPolicy;
    const gridiron::Policies policies{homePolicy, awayPolicy};

    const gridiron::TeamProfile home{"BAL", .8, .7, .8, .7, .05, .8};
    const gridiron::TeamProfile away{"SEA", .7, .9, .9, .5, .05, .8};
    const gridiron::Matchup matchup{home, away};
    std::mt19937 rng(42);

    bool sawHomePlay = false;
    bool sawAwayPlay = false;

    gridiron::runGame(
        matchup, rng, policies,
        [&](const gridiron::GameState& before, const gridiron::PlayOutcome& outcome, const gridiron::PlayResult&) {
            if (before.possession == gridiron::Possession::Home) {
                sawHomePlay = true;
                EXPECT_EQ(outcome.play_type, gridiron::PlayType::Run);
            } else {
                sawAwayPlay = true;
                EXPECT_EQ(outcome.play_type, gridiron::PlayType::Pass);
            }
        });

    EXPECT_TRUE(sawHomePlay);
    EXPECT_TRUE(sawAwayPlay);
}

TEST(TimeoutTest, NoTimeoutsLeftDoesNotAskPoliciesOrRemoveRunoff) {
    class MustNotBeAskedPolicy : public gridiron::DefaultPolicy {
    public:
        bool callTimeout(const gridiron::GameState&, const gridiron::PlayOutcome&,
                         gridiron::Possession) const override {
            ADD_FAILURE() << "A team with no timeouts must not be asked";
            return true;
        }
    };

    const MustNotBeAskedPolicy policy;
    const gridiron::Policies policies{policy, policy};

    gridiron::GameState state;
    state.home_timeouts = 0;
    state.away_timeouts = 0;

    gridiron::PlayOutcome outcome{
        .play_type = gridiron::PlayType::Run,
        .outcome_type = gridiron::OutcomeType::Normal,
        .yards_gained = 1,
        .elapsed_seconds = 6,
        .runoff_seconds = 20,
    };

    const auto caller = gridiron::applyTimeout(state, outcome, policies);

    EXPECT_FALSE(caller.has_value());
    EXPECT_EQ(state.home_timeouts, 0);
    EXPECT_EQ(state.away_timeouts, 0);
    EXPECT_EQ(outcome.elapsed_seconds, 6);
    EXPECT_EQ(outcome.runoff_seconds, 20);
}

TEST(TimeoutTest, CalledTimeoutChargesOnlyCallerAndRemovesRunoff) {
    class AlwaysTimeoutPolicy : public gridiron::DefaultPolicy {
    public:
        bool callTimeout(const gridiron::GameState&, const gridiron::PlayOutcome&,
                         gridiron::Possession) const override {
            return true;
        }
    };

    const AlwaysTimeoutPolicy callsTimeout;
    const gridiron::DefaultPolicy declinesTimeout;

    for (const auto team : {gridiron::Possession::Home, gridiron::Possession::Away}) {
        SCOPED_TRACE(team == gridiron::Possession::Home ? "Home" : "Away");

        const gridiron::Policies policies{
            team == gridiron::Possession::Home ? callsTimeout : declinesTimeout,
            team == gridiron::Possession::Away ? callsTimeout : declinesTimeout,
        };

        gridiron::GameState state;
        state.possession = gridiron::Possession::Home;
        state.home_timeouts = 2;
        state.away_timeouts = 2;

        gridiron::PlayOutcome outcome{
            .play_type = gridiron::PlayType::Run,
            .outcome_type = gridiron::OutcomeType::Normal,
            .yards_gained = 1,
            .elapsed_seconds = 6,
            .runoff_seconds = 20,
        };

        const auto caller = gridiron::applyTimeout(state, outcome, policies);

        ASSERT_TRUE(caller.has_value());
        EXPECT_EQ(*caller, team);
        EXPECT_EQ(state.home_timeouts, team == gridiron::Possession::Home ? 1 : 2);
        EXPECT_EQ(state.away_timeouts, team == gridiron::Possession::Away ? 1 : 2);
        EXPECT_EQ(outcome.elapsed_seconds, 6);
        EXPECT_EQ(outcome.runoff_seconds, 0);
        EXPECT_EQ(state.time_remaining_seconds, 900);
    }
}

TEST(GameRunTest, AppliesAndReportsCalledTimeouts) {
    class AlwaysTimeoutPolicy : public gridiron::DefaultPolicy {
    public:
        bool callTimeout(const gridiron::GameState&, const gridiron::PlayOutcome&,
                         gridiron::Possession) const override {
            return true;
        }
    };

    const AlwaysTimeoutPolicy policy;
    const gridiron::Policies policies{policy, policy};

    const gridiron::TeamProfile home{"BAL", .8, .7, .8, .7, .05, .8};
    const gridiron::TeamProfile away{"SEA", .7, .9, .9, .5, .05, .8};
    const gridiron::Matchup matchup{home, away};
    std::mt19937 rng(42);
    int timeoutsObserved = 0;

    gridiron::runGame(matchup, rng, policies,
                      [&](const gridiron::GameState& before, const gridiron::PlayOutcome& outcome,
                          const gridiron::PlayResult& result) {
                          if (!result.timeout_called.has_value()) {
                              return;
                          }
                          ++timeoutsObserved;
                          const bool homeCalled = *result.timeout_called == gridiron::Possession::Home;

                          EXPECT_EQ(result.state.home_timeouts, before.home_timeouts - (homeCalled ? 1 : 0));
                          EXPECT_EQ(result.state.away_timeouts, before.away_timeouts - (homeCalled ? 0 : 1));
                          EXPECT_EQ(outcome.runoff_seconds, 0);
                          EXPECT_EQ(result.state.time_remaining_seconds,
                                    before.time_remaining_seconds - outcome.elapsed_seconds);
                          EXPECT_EQ(result.period, gridiron::PeriodEvent::None);
                      });
    EXPECT_GT(timeoutsObserved, 0);
}

TEST(TimeoutTest, IncompletePassDoesNotAskPoliciesOrSpendTimeout) {
    class MustNotBeAskedPolicy : public gridiron::DefaultPolicy {
    public:
        bool callTimeout(const gridiron::GameState&, const gridiron::PlayOutcome&,
                         gridiron::Possession) const override {
            ADD_FAILURE() << "The clock already stops after an incompletion";
            return true;
        }
    };

    const MustNotBeAskedPolicy policy;
    const gridiron::Policies policies{policy, policy};

    gridiron::GameState state;

    gridiron::PlayOutcome outcome{
        .play_type = gridiron::PlayType::Pass,
        .outcome_type = gridiron::OutcomeType::Incomplete,
        .elapsed_seconds = 6,
        .runoff_seconds = 20,
    };

    const auto caller = gridiron::applyTimeout(state, outcome, policies);

    EXPECT_FALSE(caller.has_value());
    EXPECT_EQ(state.home_timeouts, 3);
    EXPECT_EQ(state.away_timeouts, 3);
    EXPECT_EQ(outcome.runoff_seconds, 20);
}

TEST(OvertimeTest, StartPreservesScoresAndResetsSituation) {
    gridiron::GameState state;
    state.quarter = 4;
    state.time_remaining_seconds = 0;
    state.home_score = 24;
    state.away_score = 24;
    state.home_timeouts = 0;
    state.away_timeouts = 1;
    state.down = 4;
    state.distance = 2;
    state.ball_position = 80;
    state.opening_possession = gridiron::Possession::Away;
    state.game_over = true;

    std::mt19937 rng(42);
    gridiron::RandomSource random{rng};

    gridiron::startOvertime(state, random);

    EXPECT_EQ(state.quarter, 5);
    EXPECT_EQ(state.time_remaining_seconds, 600);
    EXPECT_EQ(state.home_timeouts, 2);
    EXPECT_EQ(state.away_timeouts, 2);
    EXPECT_EQ(state.overtime_possessions_completed, 0);
    EXPECT_FALSE(state.game_over);

    EXPECT_EQ(state.home_score, 24);
    EXPECT_EQ(state.away_score, 24);
    EXPECT_EQ(state.opening_possession, gridiron::Possession::Away);

    EXPECT_EQ(state.down, 1);
    EXPECT_EQ(state.distance, 10);
    EXPECT_EQ(state.ball_position, 25);
}

TEST(OvertimeTest, FirstFieldGoalAllowsReplyAndTyingReplyContinues) {
    gridiron::GameState state;
    state.quarter = 5;
    state.time_remaining_seconds = 600;
    state.home_score = 24;
    state.away_score = 24;
    state.possession = gridiron::Possession::Home;
    state.down = 4;
    state.ball_position = 75;

    const gridiron::PlayOutcome fieldGoal{
        .play_type = gridiron::PlayType::FieldGoalAttempt,
        .outcome_type = gridiron::OutcomeType::FieldGoalMade,
        .elapsed_seconds = 6,
        .runoff_seconds = 20,
    };

    const auto beforeFirst = state;
    auto firstSummary = gridiron::applyOutcomeInPlace(state, fieldGoal);
    gridiron::finishOvertimePlay(beforeFirst, state, firstSummary);

    EXPECT_EQ(state.home_score, 27);
    EXPECT_EQ(state.away_score, 24);
    EXPECT_EQ(state.possession, gridiron::Possession::Away);
    EXPECT_EQ(state.overtime_possessions_completed, 1);
    EXPECT_FALSE(state.game_over);
    EXPECT_EQ(firstSummary.period, gridiron::PeriodEvent::None);

    // Set up the end of the reply drive without simulating its earlier plays.
    state.down = 4;
    state.ball_position = 75;

    const auto beforeReply = state;
    auto replySummary = gridiron::applyOutcomeInPlace(state, fieldGoal);
    gridiron::finishOvertimePlay(beforeReply, state, replySummary);

    EXPECT_EQ(state.home_score, 27);
    EXPECT_EQ(state.away_score, 27);
    EXPECT_EQ(state.possession, gridiron::Possession::Home);
    EXPECT_EQ(state.overtime_possessions_completed, 2);
    EXPECT_FALSE(state.game_over);
    EXPECT_EQ(replySummary.period, gridiron::PeriodEvent::None);
}

TEST(OvertimeTest, NextScoreWinsAfterBothTeamsCompletePossession) {
    gridiron::GameState state;
    state.quarter = 5;
    state.time_remaining_seconds = 300;
    state.home_score = 27;
    state.away_score = 27;
    state.overtime_possessions_completed = 2;
    state.possession = gridiron::Possession::Home;
    state.down = 4;
    state.ball_position = 75;

    const gridiron::PlayOutcome fieldGoal{
        .play_type = gridiron::PlayType::FieldGoalAttempt,
        .outcome_type = gridiron::OutcomeType::FieldGoalMade,
        .elapsed_seconds = 6,
        .runoff_seconds = 20,
    };

    const auto before = state;
    auto summary = gridiron::applyOutcomeInPlace(state, fieldGoal);
    gridiron::finishOvertimePlay(before, state, summary);

    EXPECT_EQ(state.home_score, 30);
    EXPECT_EQ(state.away_score, 27);
    EXPECT_TRUE(state.game_over);
    EXPECT_EQ(summary.period, gridiron::PeriodEvent::EndofGame);
    EXPECT_EQ(state.quarter, 5);
    EXPECT_EQ(state.time_remaining_seconds, 294);
}

TEST(OvertimeTest, SafetyOnOpeningPossessionEndsGame) {
    gridiron::GameState state;
    state.quarter = 5;
    state.time_remaining_seconds = 600;
    state.home_score = 24;
    state.away_score = 24;
    state.possession = gridiron::Possession::Home;
    state.ball_position = 1;

    const gridiron::PlayOutcome safety{
        .play_type = gridiron::PlayType::Run,
        .outcome_type = gridiron::OutcomeType::Normal,
        .yards_gained = -2,
        .elapsed_seconds = 6,
        .runoff_seconds = 20,
    };

    const auto before = state;
    auto summary = gridiron::applyOutcomeInPlace(state, safety);
    gridiron::finishOvertimePlay(before, state, summary);

    EXPECT_EQ(summary.event, gridiron::PlayEvent::Safety);
    EXPECT_EQ(state.home_score, 24);
    EXPECT_EQ(state.away_score, 26);
    EXPECT_TRUE(state.game_over);
    EXPECT_EQ(summary.period, gridiron::PeriodEvent::EndofGame);
    EXPECT_EQ(state.quarter, 5);
    EXPECT_EQ(state.time_remaining_seconds, 594);
}

TEST(OvertimeTest, FailedReplyAfterFieldGoalEndsGame) {
    gridiron::GameState state;
    state.quarter = 5;
    state.time_remaining_seconds = 300;
    state.home_score = 27;
    state.away_score = 24;
    state.overtime_possessions_completed = 1;
    state.possession = gridiron::Possession::Away;
    state.down = 4;
    state.distance = 5;
    state.ball_position = 50;

    const gridiron::PlayOutcome shortRun{
        .play_type = gridiron::PlayType::Run,
        .outcome_type = gridiron::OutcomeType::Normal,
        .yards_gained = 2,
        .elapsed_seconds = 6,
        .runoff_seconds = 20,
    };

    const auto before = state;
    auto summary = gridiron::applyOutcomeInPlace(state, shortRun);
    gridiron::finishOvertimePlay(before, state, summary);

    EXPECT_EQ(summary.turnover, gridiron::TurnoverKind::TurnoverOnDowns);
    EXPECT_EQ(state.possession, gridiron::Possession::Home);
    EXPECT_EQ(state.overtime_possessions_completed, 2);
    EXPECT_EQ(state.home_score, 27);
    EXPECT_EQ(state.away_score, 24);
    EXPECT_TRUE(state.game_over);
    EXPECT_EQ(summary.period, gridiron::PeriodEvent::EndofGame);
}

TEST(OvertimeTest, OpeningTouchdownAllowsReply) {
    gridiron::GameState state;
    state.quarter = 5;
    state.time_remaining_seconds = 600;
    state.home_score = 24;
    state.away_score = 24;
    state.possession = gridiron::Possession::Home;
    state.ball_position = 95;

    const gridiron::PlayOutcome touchdown{
        .play_type = gridiron::PlayType::Run,
        .outcome_type = gridiron::OutcomeType::Normal,
        .yards_gained = 5,
        .elapsed_seconds = 6,
    };

    const auto before = state;
    auto summary = gridiron::applyOutcomeInPlace(state, touchdown);
    gridiron::finishOvertimePlay(before, state, summary);

    EXPECT_EQ(summary.event, gridiron::PlayEvent::TouchDown);
    EXPECT_EQ(state.home_score, 31);
    EXPECT_EQ(state.away_score, 24);
    EXPECT_EQ(state.possession, gridiron::Possession::Away);
    EXPECT_EQ(state.overtime_possessions_completed, 1);
    EXPECT_FALSE(state.game_over);
    EXPECT_EQ(summary.period, gridiron::PeriodEvent::None);
}
