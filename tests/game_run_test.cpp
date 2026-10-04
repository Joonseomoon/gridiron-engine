#include <gtest/gtest.h>
#include <random>


#include "../src/engine/GameRunner.h"

TEST(GameRunTest, GameOverTrue){
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    std::mt19937 rng(2000);

    gridiron::GameState playGame = gridiron::runGame (home, away, rng);

    EXPECT_TRUE(playGame.game_over);
    EXPECT_EQ(playGame.quarter, 4);
    EXPECT_EQ(playGame.time_remaining_seconds, 0);
}

TEST(GameRunTest, SameSeedFinalState){
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    std::mt19937 rng1(2000);
    std::mt19937 rng2(2000);

    gridiron::GameState playGame1 = gridiron::runGame (home, away, rng1);
    gridiron::GameState playGame2 = gridiron::runGame (home, away, rng2);

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

TEST(GameRunTest, DifferentSeedFinalState){
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    std::mt19937 rng1(2000);
    std::mt19937 rng2(4891);

    gridiron::GameState playGame1 = gridiron::runGame (home, away, rng1);
    gridiron::GameState playGame2 = gridiron::runGame (home, away, rng2);

    EXPECT_NE(playGame1.home_score, playGame2.home_score);
    EXPECT_NE(playGame1.away_score, playGame2.away_score);
    EXPECT_NE(playGame1.distance, playGame2.distance);
    EXPECT_NE(playGame1.ball_position, playGame2.ball_position);
    EXPECT_EQ(playGame1.quarter, playGame2.quarter);
    EXPECT_EQ(playGame1.time_remaining_seconds, playGame2.time_remaining_seconds);
    EXPECT_NE(rng1, rng2);
}

TEST(GameRunTest, RepeatAfterUnrelated){
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    std::mt19937 rng1(2000);
    std::mt19937 rng2(4891);
    std::mt19937 rng3(2000);

    gridiron::GameState playGame1 = gridiron::runGame (home, away, rng1);
    gridiron::GameState playGame2 = gridiron::runGame (home, away, rng2);
    gridiron::GameState playGame3 = gridiron::runGame (home, away, rng3);

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


TEST(GameRunTest, MultiSeedTermination){
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    for (unsigned seed : {2000u, 3500u, 5420u, 1290u, 9832u}) {
        SCOPED_TRACE(seed);
        std::mt19937 rng(seed);
        const auto state = gridiron::runGame(home, away, rng);
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


TEST(GameRunTest, OpeningCoinToss){
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    bool sawHome = false;
    bool sawAway = false;
    for (unsigned i = 0; i < 50; ++i) {

        std::mt19937 rng(i);
        gridiron::GameState playGame = gridiron::runGame (home, away, rng);
        if(playGame.opening_possession == gridiron::Possession::Home){
            sawHome = true;
        }
        else if(playGame.opening_possession == gridiron::Possession::Away){
            sawAway = true;
        }
    }
    EXPECT_TRUE(sawHome);
    EXPECT_TRUE(sawAway);
}

TEST(GameRunTest, PreserveTeamIdentity){
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    std::mt19937 rng(2000);

    gridiron::GameState playGame = gridiron::runGame (home, away, rng);

    EXPECT_EQ(playGame.home.name, home.name);
    EXPECT_EQ(playGame.home.offensive_pass_rating, home.offensive_pass_rating);
    EXPECT_EQ(playGame.home.defensive_pass_rating, home.defensive_pass_rating);
    EXPECT_EQ(playGame.home.defensive_run_rating, home.defensive_run_rating);
    EXPECT_EQ(playGame.home.offensive_run_rating, home.offensive_run_rating);
    EXPECT_EQ(playGame.home.turnover_rate, home.turnover_rate);
    EXPECT_EQ(playGame.home.field_goal_rating, home.field_goal_rating);
    EXPECT_EQ(playGame.away.name, away.name);
    EXPECT_EQ(playGame.away.offensive_pass_rating, away.offensive_pass_rating);
    EXPECT_EQ(playGame.away.defensive_pass_rating, away.defensive_pass_rating);
    EXPECT_EQ(playGame.away.defensive_run_rating, away.defensive_run_rating);
    EXPECT_EQ(playGame.away.offensive_run_rating, away.offensive_run_rating);
    EXPECT_EQ(playGame.away.turnover_rate, away.turnover_rate);
    EXPECT_EQ(playGame.away.field_goal_rating, away.field_goal_rating);

}


TEST(GameRunTest, AdvancesCallerRng){
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    std::mt19937 rng(2000);

    gridiron::GameState playGame = gridiron::runGame (home, away, rng);

    std::mt19937 newRng(2000);

    EXPECT_NE(rng, newRng);
}

TEST(GameRunTest, ExtremeProfilesStillTerminate){
    //Best home possible team vs worst away possible team
    gridiron::TeamProfile homeBest{"BAL", 1.0, 1.0, 1.0, 1.0, 0.0, 1.0};
    gridiron::TeamProfile awayWorst{"SEA", 0.0, 0.0, 0.0, 0.0, 1.0, 0.0};
    std::mt19937 rng1(2000);

    gridiron::GameState playGameBestWorst = gridiron::runGame (homeBest, awayWorst, rng1);

    EXPECT_TRUE(playGameBestWorst.game_over);
    EXPECT_EQ(playGameBestWorst.quarter, 4);
    EXPECT_EQ(playGameBestWorst.time_remaining_seconds, 0);

    //Best away possible team vs worst home possible team
    gridiron::TeamProfile homeWorst{"SEA", 0.0, 0.0, 0.0, 0.0, 1.0, 0.0};
    gridiron::TeamProfile awayBest{"BAL", 1.0, 1.0, 1.0, 1.0, 0.0, 1.0};
    std::mt19937 rng2(2000);

    gridiron::GameState playGameWorstBest = gridiron::runGame (homeWorst, awayBest, rng2);

    EXPECT_TRUE(playGameWorstBest.game_over);
    EXPECT_EQ(playGameWorstBest.quarter, 4);
    EXPECT_EQ(playGameWorstBest.time_remaining_seconds, 0);

    //Turnover rates at highest
    gridiron::TeamProfile homeTurnover{"BAL", 0.8, 0.7, 0.8, 0.7, 1.0, 0.8};
    gridiron::TeamProfile awayTurnover{"SEA", 0.7, 0.9, 0.9, 0.5, 1.0, 0.8};
    std::mt19937 rng3(2000);

    gridiron::GameState playGameTurnover = gridiron::runGame (homeTurnover, awayTurnover, rng3);

    EXPECT_TRUE(playGameTurnover.game_over);
    EXPECT_EQ(playGameTurnover.quarter, 4);
    EXPECT_EQ(playGameTurnover.time_remaining_seconds, 0);


    //Turnover rates at lowest
    gridiron::TeamProfile homeNoTurnover{"BAL", 0.8, 0.7, 0.8, 0.7, 0.0, 0.8};
    gridiron::TeamProfile awayNoTurnover{"SEA", 0.7, 0.9, 0.9, 0.5, 0.0, 0.8};
    std::mt19937 rng4(2000);

    gridiron::GameState playGameNoTurnover = gridiron::runGame (homeNoTurnover, awayNoTurnover, rng4);

    EXPECT_TRUE(playGameNoTurnover.game_over);
    EXPECT_EQ(playGameNoTurnover.quarter, 4);
    EXPECT_EQ(playGameNoTurnover.time_remaining_seconds, 0);
}
