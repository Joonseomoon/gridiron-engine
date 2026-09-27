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
}


TEST(GameRunTest, MultiSeedTermination){
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    for (int seed : {2000, 3500, 5420, 1290, 9832}) {
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
    }
}


TEST(GameRunTest, OpeningCoinToss){
    gridiron::TeamProfile home{"BAL", 0.8, 0.7, 0.8, 0.7, 0.05, 0.8};
    gridiron::TeamProfile away{"SEA", 0.7, 0.9, 0.9, 0.5, 0.05, 0.8};
    bool sawHome = false;
    bool sawAway = false;
    for (int i = 0; i < 50; ++i) {

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
