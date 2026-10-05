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
