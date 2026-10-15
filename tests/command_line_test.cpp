#include "../src/cli/CommandLine.h"
#include <gtest/gtest.h>

#include <string>
#include <variant>
#include <vector>

namespace {

bool isError(const std::vector<std::string>& args) {
    const auto result = gridiron::parseArgs(args);
    return std::holds_alternative<std::string>(result);
}

bool isResolveError(const gridiron::Options& options) {
    const auto result = gridiron::resolveTeams(options);
    return std::holds_alternative<std::string>(result);
}
} // namespace

TEST(CommandLineTest, TwoTeamsAndSeed) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--seed", "1000"});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    EXPECT_EQ(options->home, "BAL");
    EXPECT_EQ(options->away, "SEA");
    ASSERT_TRUE(options->seed.has_value());
    EXPECT_EQ(*options->seed, 1000u);
}

TEST(CommandLineTest, RejectsSeedWithTrailingJunk) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--seed", "12abc"}));
}

TEST(CommandLineTest, TwoTeamsWithoutSeed) {
    const auto result = gridiron::parseArgs({"BAL", "SEA"});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    EXPECT_EQ(options->home, "BAL");
    EXPECT_EQ(options->away, "SEA");
    ASSERT_FALSE(options->seed.has_value());
}

TEST(CommandLineTest, SeedBeforeTeams) {
    const auto result = gridiron::parseArgs({"--seed", "7", "BAL", "SEA"});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    EXPECT_EQ(options->home, "BAL");
    EXPECT_EQ(options->away, "SEA");
    ASSERT_TRUE(options->seed.has_value());
    EXPECT_EQ(*options->seed, 7u);
}

TEST(CommandLineTest, AcceptsSmallestSeed) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--seed", "0"});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    EXPECT_EQ(options->home, "BAL");
    EXPECT_EQ(options->away, "SEA");
    ASSERT_TRUE(options->seed.has_value());
    EXPECT_EQ(*options->seed, 0u);
}

TEST(CommandLineTest, AcceptsLargestSeed) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--seed", "4294967295"});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    EXPECT_EQ(options->home, "BAL");
    EXPECT_EQ(options->away, "SEA");
    ASSERT_TRUE(options->seed.has_value());
    EXPECT_EQ(*options->seed, 4294967295u);
}

TEST(CommandLineTest, RejectsNonNumericSeed) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--seed", "abc"}));
}

TEST(CommandLineTest, RejectsNegativeSeed) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--seed", "-5"}));
}

TEST(CommandLineTest, RejectsSeedTooLarge) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--seed", "4294967296"}));
}

TEST(CommandLineTest, RejectsEmptySeed) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--seed", ""}));
}

TEST(CommandLineTest, RejectsSeedWithoutValue) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--seed"}));
}

TEST(CommandLineTest, RejectsSeedGivenTwice) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--seed", "1", "--seed", "2"}));
}

TEST(CommandLineTest, RejectsNoArguments) {
    EXPECT_TRUE(isError({}));
}

TEST(CommandLineTest, RejectsOneTeam) {
    EXPECT_TRUE(isError({"BAL"}));
}

TEST(CommandLineTest, RejectsThreeTeams) {
    EXPECT_TRUE(isError({"BAL", "SEA", "KC"}));
}

TEST(CommandLineTest, RejectsUnknownOption) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--sed", "1"});
    const auto* error = std::get_if<std::string>(&result);
    ASSERT_NE(error, nullptr);
    EXPECT_NE(error->find("unknown option"), std::string::npos);
}

TEST(CommandLineTest, ResolvesTwoKnownTeams) {
    const auto result =
        gridiron::resolveTeams(gridiron::Options{"BAL", "SEA", std::nullopt, std::nullopt, std::nullopt});
    const auto* matchup = std::get_if<gridiron::Matchup>(&result);
    ASSERT_NE(matchup, nullptr);
    EXPECT_EQ(matchup->home.name, "BAL");
    EXPECT_EQ(matchup->away.name, "SEA");
}

TEST(CommandLineTest, ResolvesTeamsCaseInsensitively) {
    const auto result =
        gridiron::resolveTeams(gridiron::Options{"bal", "sea", std::nullopt, std::nullopt, std::nullopt});
    const auto* matchup = std::get_if<gridiron::Matchup>(&result);
    ASSERT_NE(matchup, nullptr);
    EXPECT_EQ(matchup->home.name, "BAL");
    EXPECT_EQ(matchup->away.name, "SEA");
}

TEST(CommandLineTest, RejectsUnknownHomeTeam) {
    EXPECT_TRUE(isResolveError(gridiron::Options{"XYZ", "SEA", std::nullopt, std::nullopt, std::nullopt}));
}

TEST(CommandLineTest, RejectsUnknownAwayTeam) {
    EXPECT_TRUE(isResolveError(gridiron::Options{"BAL", "XYZ", std::nullopt, std::nullopt, std::nullopt}));
}

TEST(CommandLineTest, RejectsSameTeamTwice) {
    EXPECT_TRUE(isResolveError(gridiron::Options{"BAL", "BAL", std::nullopt, std::nullopt, std::nullopt}));
}

TEST(CommandLineTest, RejectsSameTeamDifferentCase) {
    EXPECT_TRUE(isResolveError(gridiron::Options{"BAL", "bal", std::nullopt, std::nullopt, std::nullopt}));
}

TEST(CommandLineTest, UnknownTeamErrorListsAvailableTeams) {
    const auto result =
        gridiron::resolveTeams(gridiron::Options{"BAL", "XYZ", std::nullopt, std::nullopt, std::nullopt});
    const auto* error = std::get_if<std::string>(&result);
    ASSERT_NE(error, nullptr);
    EXPECT_NE(error->find("BAL"), std::string::npos);
}

TEST(CommandLineTest, TwoTeamsAndGames) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--games", "100"});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    EXPECT_EQ(options->home, "BAL");
    EXPECT_EQ(options->away, "SEA");
    ASSERT_TRUE(options->games.has_value());
    EXPECT_EQ(*options->games, 100);
    EXPECT_FALSE(options->seed.has_value());
}

TEST(CommandLineTest, GamesAbsentWhenNotGiven) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--seed", "1000"});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    EXPECT_FALSE(options->games.has_value());
}

TEST(CommandLineTest, AcceptsFewestGames) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--games", std::to_string(gridiron::kMinGames)});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    ASSERT_TRUE(options->games.has_value());
    EXPECT_EQ(*options->games, gridiron::kMinGames);
}

TEST(CommandLineTest, AcceptsMostGames) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--games", std::to_string(gridiron::kMaxGames)});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    ASSERT_TRUE(options->games.has_value());
    EXPECT_EQ(*options->games, gridiron::kMaxGames);
}

TEST(CommandLineTest, GamesAndSeedTogether) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--games", "50", "--seed", "1000"});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    ASSERT_TRUE(options->games.has_value());
    EXPECT_EQ(*options->games, 50);
    ASSERT_TRUE(options->seed.has_value());
    EXPECT_EQ(*options->seed, 1000u);
}

TEST(CommandLineTest, SeedAndGamesBeforeTeams) {
    const auto result = gridiron::parseArgs({"--seed", "7", "--games", "3", "BAL", "SEA"});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    EXPECT_EQ(options->home, "BAL");
    EXPECT_EQ(options->away, "SEA");
    ASSERT_TRUE(options->games.has_value());
    EXPECT_EQ(*options->games, 3);
    ASSERT_TRUE(options->seed.has_value());
    EXPECT_EQ(*options->seed, 7u);
}

TEST(CommandLineTest, RejectsZeroGames) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--games", "0"}));
}

TEST(CommandLineTest, RejectsTooManyGames) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--games", std::to_string(gridiron::kMaxGames + 1)}));
}

TEST(CommandLineTest, RejectsNegativeGames) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--games", "-5"}));
}

TEST(CommandLineTest, RejectsNonNumericGames) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--games", "abc"}));
}

TEST(CommandLineTest, RejectsGamesWithTrailingJunk) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--games", "10x"}));
}

TEST(CommandLineTest, RejectsEmptyGames) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--games", ""}));
}

TEST(CommandLineTest, RejectsGamesWithoutValue) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--games"}));
}

TEST(CommandLineTest, RejectsGamesGivenTwice) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--games", "1", "--games", "2"}));
}

TEST(CommandLineTest, OutOfRangeGamesErrorNamesTheRange) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--games", "0"});
    const auto* error = std::get_if<std::string>(&result);
    ASSERT_NE(error, nullptr);
    EXPECT_NE(error->find(std::to_string(gridiron::kMaxGames)), std::string::npos);
}

TEST(CommandLineTest, CorrectlyStoresFilename) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--games", "5", "--out", "results.csv"});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    EXPECT_EQ(options->home, "BAL");
    EXPECT_EQ(options->away, "SEA");
    ASSERT_TRUE(options->games.has_value());
    EXPECT_EQ(*options->games, 5);
    EXPECT_FALSE(options->seed.has_value());
    ASSERT_TRUE(options->out.has_value());
    EXPECT_EQ(*options->out, "results.csv");
}

TEST(CommandLineTest, CorrectlyStoresFilenameInDifferentOrder) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--out", "results.csv", "--games", "5"});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    EXPECT_EQ(options->home, "BAL");
    EXPECT_EQ(options->away, "SEA");
    ASSERT_TRUE(options->games.has_value());
    EXPECT_EQ(*options->games, 5);
    EXPECT_FALSE(options->seed.has_value());
    ASSERT_TRUE(options->out.has_value());
    EXPECT_EQ(*options->out, "results.csv");
}

TEST(CommandLineTest, OutGivenMoreThanOnce) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--games", "5", "--out", "results.csv", "--out", "scores.csv"}));
}

TEST(CommandLineTest, OutWithoutGameTag) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--out", "results.csv"}));
}

TEST(CommandLineTest, RejectsOutWithoutFilename) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--seed", "1000", "--games", "5", "--out"}));
}

TEST(CommandLineTest, RejectsEmptyOutFilename) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--seed", "1000", "--games", "5", "--out", ""}));
}

TEST(CommandLineTest, RejectsFlagAsOutFilename) {
    EXPECT_TRUE(isError({"BAL", "SEA", "--seed", "1000", "--out", "--games", "5"}));
}

TEST(CommandLineTest, EmptyOut) {
    const auto result = gridiron::parseArgs({"BAL", "SEA", "--seed", "1000", "--games", "5"});
    const auto* options = std::get_if<gridiron::Options>(&result);
    ASSERT_NE(options, nullptr);

    EXPECT_EQ(options->home, "BAL");
    EXPECT_EQ(options->away, "SEA");
    ASSERT_TRUE(options->games.has_value());
    EXPECT_EQ(*options->games, 5);
    EXPECT_FALSE(options->out.has_value());
    ASSERT_TRUE(options->seed.has_value());
    EXPECT_EQ(*options->seed, 1000);
}
