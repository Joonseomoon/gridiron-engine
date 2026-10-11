#include "../src/cli/GameLog.h"
#include "../src/engine/ApplyOutcome.h"

#include <gtest/gtest.h>
#include <sstream>

TEST(GameLogTest, HomeTeamBallFirstAndPlay) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 1;
    before.time_remaining_seconds = 900;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 6, 28};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q1 15:00 - Game Start. BAL receives the football.\n"
                            "Q1 14:32 - BAL run for 6 yards. 2nd & 4 on BAL 31.\n");
}

TEST(GameLogTest, AwayTeamBallFirstAndPlay) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Away;
    before.quarter = 1;
    before.time_remaining_seconds = 900;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 6, 28};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q1 15:00 - Game Start. SEA receives the football.\n"
                            "Q1 14:32 - SEA run for 6 yards. 2nd & 4 on SEA 31.\n");
}

TEST(GameLogTest, RunPlay) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Away;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 2;
    before.distance = 8;
    before.ball_position = 40;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Run, gridiron::OutcomeType::Normal, 6, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - SEA run for 6 yards. 3rd & 2 on SEA 46.\n");
}

TEST(GameLogTest, PassPlay) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 2;
    before.distance = 8;
    before.ball_position = 40;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 6, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL pass for 6 yards. 3rd & 2 on BAL 46.\n");
}

TEST(GameLogTest, FirstDown) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 2;
    before.distance = 5;
    before.ball_position = 25;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 6, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL pass for 6 yards. First down! 1st & 10 on BAL 31.\n");
}

TEST(GameLogTest, FieldGoalGood) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 4;
    before.distance = 5;
    before.ball_position = 70;

    gridiron::PlayOutcome outcome{gridiron::PlayType::FieldGoalAttempt, gridiron::OutcomeType::FieldGoalMade, 0, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL field goal attempt from 47 yards: GOOD. BAL +3.\n"
                            "Q3 12:07 - SEA ball on SEA 25, 1st & 10 (kickoff).\n");
}

TEST(GameLogTest, FieldGoalMissed) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 4;
    before.distance = 5;
    before.ball_position = 70;

    gridiron::PlayOutcome outcome{gridiron::PlayType::FieldGoalAttempt, gridiron::OutcomeType::Turnover, 0, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL field goal attempt from 47 yards: MISSED. Turnover.\n"
                            "Q3 12:07 - SEA ball on SEA 30, 1st & 10.\n");
}

TEST(GameLogTest, TurnoveronDowns) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 4;
    before.distance = 5;
    before.ball_position = 50;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 0, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL pass for 0 yards. Short of the first down. Turnover on downs.\n"
                            "Q3 12:07 - SEA ball on midfield, 1st & 10.\n");
}

TEST(GameLogTest, Safety) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 3;
    before.distance = 10;
    before.ball_position = 1;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, -2, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL pass for -2 yards. Safety! SEA +2.\n"
                            "Q3 12:07 - SEA ball on SEA 25, 1st & 10 (kickoff).\n");
}

TEST(GameLogTest, FumbleNormal) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 3;
    before.distance = 10;
    before.ball_position = 35;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Run, gridiron::OutcomeType::Turnover, 5, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL fumble recovered by SEA. Turnover.\n"
                            "Q3 12:07 - SEA ball on BAL 40, 1st & 10.\n");
}

TEST(GameLogTest, FumbleTouchBack) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 3;
    before.distance = 2;
    before.ball_position = 99;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Run, gridiron::OutcomeType::Turnover, 5, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL fumble recovered by SEA. Touchback.\n"
                            "Q3 12:07 - SEA ball on SEA 25, 1st & 10 (touchback).\n");
}

TEST(GameLogTest, FumbleDefensiveTouchdown) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 3;
    before.distance = 10;
    before.ball_position = 1;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Run, gridiron::OutcomeType::Turnover, -5, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL fumble recovered by SEA. Defensive touchdown! SEA +7.\n"
                            "Q3 12:07 - BAL ball on BAL 25, 1st & 10 (kickoff).\n");
}

TEST(GameLogTest, InterceptionNormal) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 3;
    before.distance = 10;
    before.ball_position = 35;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Turnover, 5, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL pass intercepted by SEA. Turnover.\n"
                            "Q3 12:07 - SEA ball on BAL 40, 1st & 10.\n");
}

TEST(GameLogTest, InterceptionTouchBack) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 3;
    before.distance = 2;
    before.ball_position = 99;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Turnover, 5, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL pass intercepted by SEA. Touchback.\n"
                            "Q3 12:07 - SEA ball on SEA 25, 1st & 10 (touchback).\n");
}

TEST(GameLogTest, InterceptionDefensiveTouchdown) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 3;
    before.distance = 10;
    before.ball_position = 1;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Turnover, -5, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL pass intercepted by SEA. Defensive touchdown! SEA +7.\n"
                            "Q3 12:07 - BAL ball on BAL 25, 1st & 10 (kickoff).\n");
}

TEST(GameLogTest, OffenseTouchdown) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 3;
    before.distance = 10;
    before.ball_position = 99;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 5, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL pass for 5 yards. Touchdown! BAL +7.\n"
                            "Q3 12:07 - SEA ball on SEA 25, 1st & 10 (kickoff).\n");
}

TEST(GameLogTest, DownAndGoal) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 760;
    before.down = 3;
    before.distance = 3;
    before.ball_position = 88;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 5, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 12:07 - BAL pass for 5 yards. First down! 1st & goal on SEA 7.\n");
}

TEST(GameLogTest, QuarterEnd) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.possession = gridiron::Possession::Home;
    before.quarter = 3;
    before.time_remaining_seconds = 20;
    before.down = 1;
    before.distance = 10;
    before.ball_position = 35;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 5, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q3 0:00 - BAL pass for 5 yards.\n"
                            "Q3 0:00 - End of quarter.\n"
                            "Q4 15:00 - BAL ball on BAL 40, 2nd & 5.\n");
}

TEST(GameLogTest, HalftimeRegular) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.opening_possession = gridiron::Possession::Home;
    before.possession = gridiron::Possession::Home;
    before.quarter = 2;
    before.time_remaining_seconds = 20;
    before.down = 1;
    before.distance = 10;
    before.ball_position = 35;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 5, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q2 0:00 - BAL pass for 5 yards.\n"
                            "Q2 0:00 - Halftime.\n"
                            "Q3 15:00 - SEA ball on SEA 25, 1st & 10 (second-half kickoff).\n");
}

TEST(GameLogTest, HalftimeTouchdown) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.opening_possession = gridiron::Possession::Away;
    before.possession = gridiron::Possession::Home;
    before.quarter = 2;
    before.time_remaining_seconds = 20;
    before.down = 1;
    before.distance = 10;
    before.ball_position = 99;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 5, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q2 0:00 - BAL pass for 5 yards. Touchdown! BAL +7.\n"
                            "Q2 0:00 - Halftime.\n"
                            "Q3 15:00 - BAL ball on BAL 25, 1st & 10 (second-half kickoff).\n");
}

TEST(GameLogTest, EndofGame) {
    gridiron::GameState before;
    before.home.name = "BAL";
    before.away.name = "SEA";
    before.opening_possession = gridiron::Possession::Away;
    before.possession = gridiron::Possession::Home;
    before.quarter = 4;
    before.time_remaining_seconds = 20;
    before.down = 1;
    before.distance = 10;
    before.ball_position = 35;

    gridiron::PlayOutcome outcome{gridiron::PlayType::Pass, gridiron::OutcomeType::Normal, 5, 33};

    const auto result = gridiron::applyOutcome(before, outcome);

    std::ostringstream output;
    gridiron::printPlay(output, before, outcome, result);

    EXPECT_EQ(output.str(), "Q4 0:00 - BAL pass for 5 yards.\n"
                            "Q4 0:00 - Game End.\n");
}

TEST(GameLogTest, PrintFinalResultHomeWinner) {
    std::string home = "BAL";
    std::string away = "SEA";

    gridiron::SimulationResult gameResult;
    gameResult.home_score = 27;
    gameResult.away_score = 3;
    gameResult.winner = gridiron::Winner::Home;
    gameResult.seed = 2000;

    std::ostringstream output;
    gridiron::printFinalResult(output, home, away, gameResult);

    EXPECT_EQ(output.str(), "Final Score\n"
                            "BAL - 27\n"
                            "SEA - 3\n"
                            "\n"
                            "Winner: BAL\n"
                            "Seed: 2000\n");
}

TEST(GameLogTest, PrintFinalResultAwayWinner) {
    std::string home = "BAL";
    std::string away = "SEA";

    gridiron::SimulationResult gameResult;
    gameResult.home_score = 3;
    gameResult.away_score = 27;
    gameResult.winner = gridiron::Winner::Away;
    gameResult.seed = 2000;

    std::ostringstream output;
    gridiron::printFinalResult(output, home, away, gameResult);

    EXPECT_EQ(output.str(), "Final Score\n"
                            "BAL - 3\n"
                            "SEA - 27\n"
                            "\n"
                            "Winner: SEA\n"
                            "Seed: 2000\n");
}

TEST(GameLogTest, PrintFinalResultTie) {
    std::string home = "BAL";
    std::string away = "SEA";

    gridiron::SimulationResult gameResult;
    gameResult.home_score = 27;
    gameResult.away_score = 27;
    gameResult.winner = gridiron::Winner::Tie;
    gameResult.seed = 2000;

    std::ostringstream output;
    gridiron::printFinalResult(output, home, away, gameResult);

    EXPECT_EQ(output.str(), "Final Score\n"
                            "BAL - 27\n"
                            "SEA - 27\n"
                            "\n"
                            "Winner: Tie\n"
                            "Seed: 2000\n");
}
