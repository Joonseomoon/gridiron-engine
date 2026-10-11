#pragma once

#include "SimulationResult.h"
#include <vector>

namespace gridiron {

struct BatchSummary {
    int games = 0;
    int home_wins = 0;
    int away_wins = 0;
    int ties = 0;

    double mean_home_score = 0.0;
    double mean_away_score = 0.0;
    double mean_margin = 0.0;
    double mean_total = 0.0;
};

BatchSummary aggregate(const std::vector<SimulationResult>& results);
} // namespace gridiron
