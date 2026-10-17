#include "BatchLog.h"

#include <iomanip>
#include <ostream>
#include <string>

namespace gridiron {

void printBatchSummary(std::ostream& out, const std::string& homeName, const std::string& awayName,
                       const BatchSummary& summary) {

    const auto previousFlags = out.flags();
    const auto previousPrecision = out.precision();

    out << homeName << " wins: " << summary.home_wins << "\t" << awayName << " wins: " << summary.away_wins << "\t"
        << "Ties: " << summary.ties << '\n';

    out << std::fixed << std::setprecision(2);

    out << "Mean score: " << homeName << " " << summary.mean_home_score << "\t" << awayName << " "
        << summary.mean_away_score << '\n';

    out << "Mean margin (" << homeName << " - " << awayName << "): " << summary.mean_margin << "\t"
        << "Mean total: " << summary.mean_total << '\n';

    out << "Score sd: " << homeName << " " << summary.sd_home_score << "\t" << awayName << " " << summary.sd_away_score
        << "\t" << "Margin sd: " << summary.sd_margin << "\t" << "Total sd: " << summary.sd_total << '\n';

    out << "95% CI: " << homeName << " win rate " << std::setprecision(3) << summary.home_win_rate << " ± "
        << summary.home_win_rate_ci95 << std::setprecision(2) << "\t" << "Mean margin " << summary.mean_margin << " ± "
        << summary.mean_margin_ci95 << '\n';

    out.flags(previousFlags);
    out.precision(previousPrecision);
}
} // namespace gridiron
