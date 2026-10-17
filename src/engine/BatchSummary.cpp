#include "BatchSummary.h"
#include "SimulationResult.h"

#include <cmath>
#include <vector>

namespace gridiron {

namespace {
constexpr double kZ95 = 1.96;
}

BatchSummary aggregate(const std::vector<SimulationResult>& results) {
    BatchSummary totalSummary;

    if (results.empty()) {
        return totalSummary;
    }

    int totalGames = static_cast<int>(results.size());
    totalSummary.games = totalGames;

    double totalHomeScore{0.0};
    double totalAwayScore{0.0};

    for (const auto& result : results) {
        totalHomeScore += result.home_score;
        totalAwayScore += result.away_score;
        if (result.winner == Winner::Home) {
            totalSummary.home_wins += 1;
        } else if (result.winner == Winner::Away) {
            totalSummary.away_wins += 1;
        } else if (result.winner == Winner::Tie) {
            totalSummary.ties += 1;
        }
    }

    totalSummary.mean_home_score = totalHomeScore / totalGames;
    totalSummary.mean_away_score = totalAwayScore / totalGames;
    totalSummary.mean_margin = totalSummary.mean_home_score - totalSummary.mean_away_score;
    totalSummary.mean_total = totalSummary.mean_home_score + totalSummary.mean_away_score;

    totalSummary.home_win_rate = static_cast<double>(totalSummary.home_wins) / static_cast<double>(totalGames);

    // return early if fewer than 2 games to avoid dividing by n - 1 = 0 in sd
    if (totalGames < 2) {
        return totalSummary;
    }

    double homeScoreSquaredDeviation{0.0};
    double awayScoreSquaredDeviation{0.0};
    double marginSquaredDeviation{0.0};
    double totalSquaredDeviation{0.0};

    for (const auto& result : results) {
        const double homeDeviation = result.home_score - totalSummary.mean_home_score;
        homeScoreSquaredDeviation += homeDeviation * homeDeviation;

        const double awayDeviation = result.away_score - totalSummary.mean_away_score;
        awayScoreSquaredDeviation += awayDeviation * awayDeviation;

        const double margin = result.home_score - result.away_score;
        const double marginDeviation = margin - totalSummary.mean_margin;
        marginSquaredDeviation += (marginDeviation) * (marginDeviation);

        const double total = result.home_score + result.away_score;
        const double totalDeviation = total - totalSummary.mean_total;
        totalSquaredDeviation += totalDeviation * totalDeviation;
    }

    totalSummary.sd_home_score = std::sqrt(homeScoreSquaredDeviation / static_cast<double>(totalGames - 1));
    totalSummary.sd_away_score = std::sqrt(awayScoreSquaredDeviation / static_cast<double>(totalGames - 1));
    totalSummary.sd_margin = std::sqrt(marginSquaredDeviation / static_cast<double>(totalGames - 1));
    totalSummary.sd_total = std::sqrt(totalSquaredDeviation / static_cast<double>(totalGames - 1));

    totalSummary.mean_margin_ci95 = totalSummary.sd_margin / std::sqrt(static_cast<double>(totalGames)) * kZ95;
    totalSummary.home_win_rate_ci95 =
        std::sqrt(totalSummary.home_win_rate * (1.0 - totalSummary.home_win_rate) / totalGames) * kZ95;

    return totalSummary;
}

} // namespace gridiron
