#include "BatchCsv.h"
#include "../engine/BatchMetadata.h"
#include "../engine/BatchSummary.h"
#include "../engine/ModelVersion.h"
#include "../engine/SimulationResult.h"

#include <cstddef>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

namespace gridiron {

void writeBatchCsv(std::ostream& out, const BatchMetadata& metadata, const std::vector<SimulationResult>& results,
                   const BatchSummary& summary) {

    const auto originalFlags = out.flags();
    const auto originalPrecision = out.precision();
    out << "# model_version: " << kModelVersion << '\n';

    out << "# home: " << metadata.home.name << '\n';
    out << "# home_offensive_pass_rating: " << metadata.home.offensive_pass_rating << '\n';
    out << "# home_offensive_run_rating: " << metadata.home.offensive_run_rating << '\n';
    out << "# home_defensive_pass_rating: " << metadata.home.defensive_pass_rating << '\n';
    out << "# home_defensive_run_rating: " << metadata.home.defensive_run_rating << '\n';
    out << "# home_turnover_rate: " << metadata.home.turnover_rate << '\n';
    out << "# home_field_goal_rating: " << metadata.home.field_goal_rating << '\n';

    out << "# away: " << metadata.away.name << '\n';
    out << "# away_offensive_pass_rating: " << metadata.away.offensive_pass_rating << '\n';
    out << "# away_offensive_run_rating: " << metadata.away.offensive_run_rating << '\n';
    out << "# away_defensive_pass_rating: " << metadata.away.defensive_pass_rating << '\n';
    out << "# away_defensive_run_rating: " << metadata.away.defensive_run_rating << '\n';
    out << "# away_turnover_rate: " << metadata.away.turnover_rate << '\n';
    out << "# away_field_goal_rating: " << metadata.away.field_goal_rating << '\n';

    out << "# games: " << metadata.games << '\n';
    out << "# batch_seed: " << metadata.batchSeed << '\n';
    out << "# seed_policy: batchSeed + game (uint32 wrap)\n";
    out << "# command: " << metadata.commandLine << '\n';

    out << "# home_wins: " << summary.home_wins << '\n';
    out << "# away_wins: " << summary.away_wins << '\n';
    out << "# ties: " << summary.ties << '\n';

    out << std::fixed << std::setprecision(2);
    out << "# mean_home_score: " << summary.mean_home_score << '\n';
    out << "# mean_away_score: " << summary.mean_away_score << '\n';
    out << "# mean_margin: " << summary.mean_margin << '\n';
    out << "# mean_total: " << summary.mean_total << '\n';
    out << "# sd_home_score: " << summary.sd_home_score << '\n';
    out << "# sd_away_score: " << summary.sd_away_score << '\n';
    out << "# sd_margin: " << summary.sd_margin << '\n';
    out << "# sd_total: " << summary.sd_total << '\n';
    out << "# home_win_rate: " << std::setprecision(3) << summary.home_win_rate << '\n';
    out << "# home_win_rate_ci95: " << summary.home_win_rate_ci95 << '\n';
    out << "# mean_margin_ci95: " << std::setprecision(2) << summary.mean_margin_ci95 << '\n';

    out << "game,seed,home_score,away_score,winner\n";

    for (std::size_t i = 0; i < results.size(); ++i) {
        const auto& result = results[i];
        std::string winnerText;
        switch (result.winner) {
        case Winner::Home: winnerText = "home"; break;
        case Winner::Away: winnerText = "away"; break;
        case Winner::Tie: winnerText = "tie"; break;
        }
        out << i << "," << result.seed << "," << result.home_score << "," << result.away_score << "," << winnerText
            << '\n';
    }
    out.flags(originalFlags);
    out.precision(originalPrecision);
}
} // namespace gridiron
