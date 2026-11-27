#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "../src/engine/AggressivePolicy.h"
#include "../src/engine/CandidateAction.h"
#include "../src/engine/ConservativePolicy.h"
#include "../src/engine/DefaultPolicy.h"
#include "../src/engine/GameRunner.h"
#include "../src/engine/LateGamePolicy.h"

namespace {
const gridiron::TeamProfile kBal{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
const gridiron::TeamProfile kSea{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};

constexpr unsigned kBatchSeed = 1000;
constexpr int kGames = 1000;

// The M4.md example: BAL (home) trails SEA 24-27, 3:14 left, 4th and 2 at SEA's 43.
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

// BAL's average win value (win 1, tie 0.5, loss 0) after the forced first action.
// Every action uses the same seeds, kBatchSeed + i, so the actions are compared on the same games.
double balWinValue(const gridiron::GameState& start, const gridiron::Matchup& matchup,
                   const gridiron::Policies& policies, gridiron::CandidateAction action) {
    double total = 0.0;
    for (int i = 0; i < kGames; ++i) {
        std::mt19937 rng(kBatchSeed + static_cast<unsigned>(i));
        const gridiron::GameState final = gridiron::resumeGame(start, matchup, policies, action, rng);
        if (final.home_score > final.away_score) {
            total += 1.0;
        } else if (final.home_score == final.away_score) {
            total += 0.5;
        }
    }
    return total / kGames;
}

struct NamedPolicies {
    std::string name; // "BAL policy v SEA policy"
    gridiron::Policies policies;
};

struct NamedAction {
    std::string name;
    gridiron::CandidateAction action;
};

const std::vector<NamedAction> kActions{{"GoForIt", gridiron::CandidateAction::GoForIt},
                                        {"FieldGoal", gridiron::CandidateAction::FieldGoal},
                                        {"Punt", gridiron::CandidateAction::Punt}};
} // namespace

// 4.3 A: the example's actions under each pair of continuation policies (BAL listed first).
TEST(PolicySensitivityTest, ExampleActionsUnderContinuationPolicies) {
    const gridiron::Matchup matchup{.home = kBal, .away = kSea};
    const gridiron::GameState start = exampleScenario();

    const gridiron::DefaultPolicy standard;
    const gridiron::LateGamePolicy lateGame;
    const gridiron::ConservativePolicy conservative;
    const gridiron::AggressivePolicy aggressive;
    const std::vector<NamedPolicies> pairs{
        {"Default v Default", {standard, standard}},
        {"LateGame v LateGame", {lateGame, lateGame}},
        {"Conservative v Conservative", {conservative, conservative}},
        {"Aggressive v Aggressive", {aggressive, aggressive}},
        {"Aggressive v Conservative", {aggressive, conservative}},
        {"Conservative v Aggressive", {conservative, aggressive}},
    };

    std::cout << std::fixed << std::setprecision(3);
    for (const auto& pair : pairs) {
        std::cout << std::left << std::setw(30) << pair.name;
        for (const auto& [name, action] : kActions) {
            const double value = balWinValue(start, matchup, pair.policies, action);
            EXPECT_GE(value, 0.0);
            EXPECT_LE(value, 1.0);
            std::cout << name << ' ' << value << "  ";
        }
        std::cout << '\n';
    }
}

// 4.3 A input sensitivity: BAL's field-goal rating, under LateGamePolicy for both teams.
// From SEA's 43 the kick is 60 yards, so the make chance is rating - 0.40.
TEST(PolicySensitivityTest, ExampleActionsUnderBalFieldGoalRating) {
    const gridiron::GameState start = exampleScenario();
    const gridiron::LateGamePolicy lateGame;
    const gridiron::Policies policies{lateGame, lateGame};

    std::cout << std::fixed << std::setprecision(3);
    for (double rating : {0.6, 0.8, 1.0}) {
        gridiron::TeamProfile bal = kBal;
        bal.field_goal_rating = rating;
        const gridiron::Matchup matchup{.home = bal, .away = kSea};

        std::cout << "BAL field_goal_rating " << std::setprecision(1) << rating << std::setprecision(3) << "     ";
        for (const auto& [name, action] : kActions) {
            const double value = balWinValue(start, matchup, policies, action);
            EXPECT_GE(value, 0.0);
            EXPECT_LE(value, 1.0);
            std::cout << name << ' ' << value << "  ";
        }
        std::cout << '\n';
    }
}
