#include <gtest/gtest.h>
#include <random>
#include <vector>

#include "../src/engine/GameRunner.h"

// M1.2 shared checkpoint: run many complete games across seeds and team
// matchups, and check every intermediate state against the legal-state rules
// in docs/DECISIONS.md — not just the final state.

namespace {

struct Matchup {
    const char* label;
    gridiron::TeamProfile home;
    gridiron::TeamProfile away;
};

const std::vector<Matchup>& matchups() {
    static const std::vector<Matchup> all = {
        {"typical", {"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.95}, {"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.95}},
        {"best home vs worst away", {"BAL", 1.0, 1.0, 1.0, 1.0, 0.0, 1.0}, {"SEA", 0.0, 0.0, 0.0, 0.0, 1.0, 0.0}},
        {"worst home vs best away", {"BAL", 0.0, 0.0, 0.0, 0.0, 1.0, 0.0}, {"SEA", 1.0, 1.0, 1.0, 1.0, 0.0, 1.0}},
        {"turnover heavy", {"BAL", 0.8, 0.7, 0.8, 0.7, 0.3, 0.95}, {"SEA", 0.7, 0.9, 0.9, 0.5, 0.3, 0.95}},
        // Both offenses stall, so most drives end on 4th down: exercises the
        // field goal decision and turnover on downs heavily.
        {"defensive struggle", {"BAL", 0.2, 0.2, 0.9, 0.9, 0.02, 0.9}, {"SEA", 0.2, 0.2, 0.9, 0.9, 0.02, 0.9}},
    };
    return all;
}

constexpr int kSeedsPerMatchup = 1000;

void expectLegal(const gridiron::GameState& s) {
    EXPECT_GE(s.down, 1);
    EXPECT_LE(s.down, 4);
    EXPECT_GE(s.distance, 1);
    // The first down marker can never be past the goal line.
    EXPECT_LE(s.distance, 100 - s.ball_position);
    EXPECT_GE(s.ball_position, 1);
    EXPECT_LE(s.ball_position, 99);
    EXPECT_GE(s.quarter, 1);
    EXPECT_LE(s.quarter, 4);
    EXPECT_GE(s.time_remaining_seconds, 0);
    EXPECT_LE(s.time_remaining_seconds, 900);
    EXPECT_GE(s.home_score, 0);
    EXPECT_GE(s.away_score, 0);
}

gridiron::Possession opponentOf(gridiron::Possession p) {
    return p == gridiron::Possession::Home ? gridiron::Possession::Away : gridiron::Possession::Home;
}

} // namespace

TEST(InvariantSweepTest, EveryIntermediateStateIsLegal) {
    for (const Matchup& m : matchups()) {
        for (int seed = 0; seed < kSeedsPerMatchup; ++seed) {
            SCOPED_TRACE(testing::Message() << m.label << ", seed " << seed);
            std::mt19937 rng(seed);

            int plays = 0;
            int homeFromScoringPlays = 0;
            int awayFromScoringPlays = 0;

            const auto onPlay = [&](const gridiron::GameState& before, const gridiron::PlayOutcome& outcome,
                                    const gridiron::GameState& after) {
                ++plays;
                expectLegal(before);
                if (!after.game_over) {
                    expectLegal(after);
                }
                EXPECT_FALSE(before.game_over);
                EXPECT_GE(outcome.elapsed_seconds, 10);

                // Field goal decision rule: kick exactly when it is 4th down
                // within 40 yards of the goal line.
                const bool inFieldGoalSpot = before.down == 4 && 100 - before.ball_position <= 40;
                EXPECT_EQ(outcome.play_type == gridiron::PlayType::FieldGoalAttempt, inFieldGoalSpot);

                // At most one team scores per play, and only by 2, 3, or 7.
                const int homeDelta = after.home_score - before.home_score;
                const int awayDelta = after.away_score - before.away_score;
                EXPECT_TRUE(homeDelta == 0 || awayDelta == 0);
                for (int delta : {homeDelta, awayDelta}) {
                    EXPECT_TRUE(delta == 0 || delta == 2 || delta == 3 || delta == 7) << "delta " << delta;
                }
                homeFromScoringPlays += homeDelta;
                awayFromScoringPlays += awayDelta;
                const bool scored = homeDelta != 0 || awayDelta != 0;

                // Clock only moves forward: same quarter with less time, or
                // the next quarter with a fresh 900 seconds.
                if (after.quarter == before.quarter && !after.game_over) {
                    EXPECT_LT(after.time_remaining_seconds, before.time_remaining_seconds);
                } else if (!after.game_over) {
                    EXPECT_EQ(after.quarter, before.quarter + 1);
                    EXPECT_EQ(after.time_remaining_seconds, 900);
                }

                // After any score the receiving team starts 1st and 10 at its 25.
                if (scored) {
                    EXPECT_EQ(after.down, 1);
                    EXPECT_EQ(after.distance, 10);
                    EXPECT_EQ(after.ball_position, 25);
                }

                // Halftime hands the ball to the team that did not open,
                // unless the last play of the half scored and set its own kickoff.
                if (before.quarter == 2 && after.quarter == 3 && !scored) {
                    EXPECT_EQ(after.possession, opponentOf(after.opening_possession));
                    EXPECT_EQ(after.ball_position, 25);
                }

                EXPECT_EQ(after.opening_possession, before.opening_possession);
            };

            const gridiron::GameState final = gridiron::runGame(m.home, m.away, rng, onPlay);

            EXPECT_TRUE(final.game_over);
            EXPECT_EQ(final.quarter, 4);
            EXPECT_EQ(final.time_remaining_seconds, 0);
            EXPECT_GT(plays, 0);
            // Final score equals the sum of the scoring plays observed.
            EXPECT_EQ(final.home_score, homeFromScoringPlays);
            EXPECT_EQ(final.away_score, awayFromScoringPlays);

            if (testing::Test::HasFailure()) {
                return; // One failing game is enough to diagnose; don't flood the output.
            }
        }
    }
}

// Attaching an observer must not change the game: it draws no random numbers.
TEST(InvariantSweepTest, ObserverDoesNotChangeTheGame) {
    const Matchup& m = matchups().front();
    for (int seed = 0; seed < 50; ++seed) {
        SCOPED_TRACE(seed);
        std::mt19937 rngPlain(seed);
        std::mt19937 rngObserved(seed);

        const auto plain = gridiron::runGame(m.home, m.away, rngPlain);
        const auto observed = gridiron::runGame(m.home, m.away, rngObserved,
                                                [](const auto&, const auto&, const auto&) {});

        EXPECT_EQ(plain.home_score, observed.home_score);
        EXPECT_EQ(plain.away_score, observed.away_score);
        EXPECT_EQ(plain.opening_possession, observed.opening_possession);
        EXPECT_EQ(rngPlain, rngObserved);
    }
}

// Same seed, same teams reproduces an identical play sequence, not just an
// identical final score.
TEST(InvariantSweepTest, SameSeedReproducesEveryPlay) {
    const Matchup& m = matchups().front();
    const auto record = [&](int seed) {
        std::vector<gridiron::PlayOutcome> plays;
        std::mt19937 rng(seed);
        gridiron::runGame(m.home, m.away, rng,
                          [&](const auto&, const gridiron::PlayOutcome& outcome, const auto&) {
                              plays.push_back(outcome);
                          });
        return plays;
    };

    const auto first = record(1000);
    const auto second = record(1000);
    ASSERT_EQ(first.size(), second.size());
    for (size_t i = 0; i < first.size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(first[i].play_type, second[i].play_type);
        EXPECT_EQ(first[i].outcome_type, second[i].outcome_type);
        EXPECT_EQ(first[i].yards_gained, second[i].yards_gained);
        EXPECT_EQ(first[i].elapsed_seconds, second[i].elapsed_seconds);
    }
}
