#include "BatchSummary.h"
#include "SimulationResult.h"

#include <vector>

namespace gridiron {

BatchSummary aggregate(const std::vector<SimulationResult>& results) {
    BatchSummary totalSummary;

    if (results.empty()) {
        return totalSummary;
    }

    int totalGames = static_cast<int>(results.size());
    totalSummary.games = totalGames;

    double totalHomeScore{0.0};
    double totalAwayScore{0.0};

    for (std::size_t i = 0; i < results.size(); ++i) {
        totalHomeScore += results[i].home_score;
        totalAwayScore += results[i].away_score;
        if (results[i].winner == Winner::Home) {
            totalSummary.home_wins += 1;
        } else if (results[i].winner == Winner::Away) {
            totalSummary.away_wins += 1;
        } else if (results[i].winner == Winner::Tie) {
            totalSummary.ties += 1;
        }
    }

    totalSummary.mean_home_score = totalHomeScore / totalGames;
    totalSummary.mean_away_score = totalAwayScore / totalGames;
    totalSummary.mean_margin = totalSummary.mean_home_score - totalSummary.mean_away_score;
    totalSummary.mean_total = totalSummary.mean_home_score + totalSummary.mean_away_score;

    return totalSummary;
}

} // namespace gridiron