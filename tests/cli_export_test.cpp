#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

class CliExportTest : public ::testing::Test {
protected:
    std::filesystem::path directory;

    void SetUp() override {
        const auto pattern = (std::filesystem::temp_directory_path() / "gridiron-export-test-XXXXXX").string();

        std::vector<char> buffer(pattern.begin(), pattern.end());
        buffer.push_back('\0');

        const char* created = mkdtemp(buffer.data());
        ASSERT_NE(created, nullptr);
        directory = created;
    }

    void TearDown() override {
        if (!directory.empty()) {
            std::error_code error;
            std::filesystem::remove_all(directory, error);
        }
    }
};

std::string shellQuote(const std::string& argument) {
    std::string quoted = "'";
    for (char character : argument) {
        if (character == '\'') {
            quoted += "'\\''";
        } else {
            quoted += character;
        }
    }

    quoted += "'";
    return quoted;
}

TEST_F(CliExportTest, CreatesCsvFile) {
    const auto outputPath = directory / "results.csv";

    const std::string command =
        shellQuote(SIMULATE_PATH) + " BAL SEA --games 3 --seed 1000 --out " + shellQuote(outputPath.string());

    const int status = std::system(command.c_str());
    ASSERT_EQ(status, 0);

    std::ifstream file(outputPath);
    ASSERT_TRUE(file.is_open());

    std::string firstLine;
    ASSERT_TRUE(static_cast<bool>(std::getline(file, firstLine)));
    EXPECT_EQ(firstLine, "# model_version: m1-v1");
}

TEST_F(CliExportTest, RejectsMissingParentDirectory) {
    const auto outputPath = directory / "missing" / "results.csv";
    const auto errorPath = directory / "stderr.txt";

    const std::string command = shellQuote(SIMULATE_PATH) + " BAL SEA --games 3 --seed 1000 --out " +
                                shellQuote(outputPath.string()) + " 2> " + shellQuote(errorPath.string());

    const int status = std::system(command.c_str());
    ASSERT_NE(status, -1);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 1);
    EXPECT_FALSE(std::filesystem::exists(outputPath));

    std::ifstream errorFile(errorPath);
    ASSERT_TRUE(errorFile.is_open());

    std::string errorLine;
    ASSERT_TRUE(static_cast<bool>(std::getline(errorFile, errorLine)));
    EXPECT_NE(errorLine.find("error:"), std::string::npos);
}

TEST_F(CliExportTest, CleansUpTemporaryFileWhenRenameFails) {
    const auto outputPath = directory / "existing-directory";
    ASSERT_TRUE(std::filesystem::create_directory(outputPath));

    const std::string command =
        shellQuote(SIMULATE_PATH) + " BAL SEA --games 3 --seed 1000 --out " + shellQuote(outputPath.string());

    const int status = std::system(command.c_str());

    ASSERT_NE(status, -1);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 1);
    EXPECT_TRUE(std::filesystem::is_directory(outputPath));
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        EXPECT_EQ(entry.path(), outputPath);
    }
}

TEST_F(CliExportTest, RecordedCommandReproducedIdenticalCsv) {
    const auto outputPath = directory / "Austin's results.csv";

    const std::string command =
        shellQuote(SIMULATE_PATH) + " BAL SEA --games 3 --out " + shellQuote(outputPath.string());

    ASSERT_EQ(std::system(command.c_str()), 0);

    std::ifstream firstFile(outputPath, std::ios::binary);
    ASSERT_TRUE(firstFile.is_open());

    const std::string firstContents{std::istreambuf_iterator<char>(firstFile), std::istreambuf_iterator<char>()};
    firstFile.close();

    const std::string prefix = "# command: ";
    std::istringstream lines(firstContents);
    std::string line;
    std::string replayCommand;

    while (std::getline(lines, line)) {
        if (line.starts_with(prefix)) {
            replayCommand = line.substr(prefix.size());
            break;
        }
    }

    ASSERT_FALSE(replayCommand.empty());

    ASSERT_EQ(std::system(replayCommand.c_str()), 0);

    std::ifstream secondFile(outputPath, std::ios::binary);
    ASSERT_TRUE(secondFile.is_open());

    const std::string secondContents{std::istreambuf_iterator<char>(secondFile), std::istreambuf_iterator<char>()};
    EXPECT_EQ(firstContents, secondContents);
}
