#include "../src/cli/Teams.h"
#include <gtest/gtest.h>
#include <set>

TEST(TeamsTest, FindsKnownTeam) {
    const auto teamProfile = gridiron::findTeam("BAL");
    ASSERT_TRUE(teamProfile.has_value());
    EXPECT_EQ("BAL", teamProfile->name);
    EXPECT_DOUBLE_EQ(0.8, teamProfile->offensive_pass_rating);
    EXPECT_DOUBLE_EQ(0.8, teamProfile->offensive_run_rating);
    EXPECT_DOUBLE_EQ(0.8, teamProfile->defensive_pass_rating);
    EXPECT_DOUBLE_EQ(0.7, teamProfile->defensive_run_rating);
    EXPECT_DOUBLE_EQ(0.03, teamProfile->turnover_rate);
    EXPECT_DOUBLE_EQ(0.94, teamProfile->field_goal_rating);
}

TEST(TeamsTest, FindsKnownTeamLowerCaseInput) {
    const auto teamProfile1 = gridiron::findTeam("bal");
    ASSERT_TRUE(teamProfile1.has_value());
    EXPECT_EQ("BAL", teamProfile1->name);

    const auto teamProfile2 = gridiron::findTeam("Bal");
    ASSERT_TRUE(teamProfile2.has_value());
    EXPECT_EQ("BAL", teamProfile2->name);
}

TEST(TeamsTest, UnknownTeamReturnsEmpty) {
    const auto teamProfile = gridiron::findTeam("XYZ");
    ASSERT_FALSE(teamProfile.has_value());
}

TEST(TeamsTest, EveryListedAbbreviationIsFound) {
    const auto abbreviations = gridiron::teamAbbreviations();
    ASSERT_FALSE(abbreviations.empty());

    for (const auto& abbreviation : abbreviations) {
        SCOPED_TRACE(abbreviation);
        const auto teamProfile = gridiron::findTeam(abbreviation);
        ASSERT_TRUE(teamProfile.has_value());
        EXPECT_EQ(abbreviation, teamProfile->name);
    }
}

TEST(TeamsTest, AllRatingsAreInRange) {
    const auto abbreviations = gridiron::teamAbbreviations();
    ASSERT_FALSE(abbreviations.empty());

    for (const auto& abbreviation : abbreviations) {
        SCOPED_TRACE(abbreviation);
        const auto teamProfile = gridiron::findTeam(abbreviation);
        ASSERT_TRUE(teamProfile.has_value());
        EXPECT_GE(teamProfile->offensive_pass_rating, 0.0);
        EXPECT_LE(teamProfile->offensive_pass_rating, 1.0);
        EXPECT_GE(teamProfile->offensive_run_rating, 0.0);
        EXPECT_LE(teamProfile->offensive_run_rating, 1.0);
        EXPECT_GE(teamProfile->defensive_pass_rating, 0.0);
        EXPECT_LE(teamProfile->defensive_pass_rating, 1.0);
        EXPECT_GE(teamProfile->defensive_run_rating, 0.0);
        EXPECT_LE(teamProfile->defensive_run_rating, 1.0);
        EXPECT_GE(teamProfile->turnover_rate, 0.0);
        EXPECT_LE(teamProfile->turnover_rate, 1.0);
        EXPECT_GE(teamProfile->field_goal_rating, 0.0);
        EXPECT_LE(teamProfile->field_goal_rating, 1.0);
    }
}

TEST(TeamsTest, AbbreviationsAreUnique) {
    const auto abbreviations = gridiron::teamAbbreviations();
    ASSERT_FALSE(abbreviations.empty());
    const std::set<std::string> unique(abbreviations.begin(), abbreviations.end());
    EXPECT_EQ(unique.size(), abbreviations.size());
}
