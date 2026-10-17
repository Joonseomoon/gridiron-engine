#include "BatchSummary.h"
#include "SimulationResult.h"

#include <cmath>
#include <vector>

namespace gridiron {

namespace {
constexpr double kZ95 = 1.96;

// 95% CI half-width for a win rate of wins / games (normal approximation); 0 with fewer than 2 games.
double winRateCi95(int wins, int games) {
    if (games < 2) {
        return 0.0;
    }
    const double n = games;
    const double p = static_cast<double>(wins) / n;
    return kZ95 * std::sqrt(p * (1.0 - p) / n);
}
} // namespace

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
    totalSummary.home_win_rate_ci95 = winRateCi95(totalSummary.home_wins, totalGames);

    return totalSummary;
}

std::vector<ConvergencePoint> convergence(const std::vector<SimulationResult>& results) {
    std::vector<ConvergencePoint> convergencePoints;

    int homeWins{0};
    std::size_t nextStop = 10;

    for (std::size_t i = 0; i < results.size(); i++) {
        if (results[i].winner == Winner::Home) {
            homeWins += 1;
        }
        if (i == results.size() - 1 || i + 1 == nextStop) {
            ConvergencePoint convergencePoint;
            convergencePoint.games = static_cast<int>(i + 1);
            const double games = convergencePoint.games;
            convergencePoint.home_win_rate = static_cast<double>(homeWins) / games;
            convergencePoint.ci95 = winRateCi95(homeWins, convergencePoint.games);
            convergencePoints.push_back(convergencePoint);
            nextStop *= 10;
        }
    }
    return convergencePoints;
}
} // namespace gridiron
