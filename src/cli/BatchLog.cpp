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

    out.flags(previousFlags);
    out.precision(previousPrecision);
}
} // namespace gridiron