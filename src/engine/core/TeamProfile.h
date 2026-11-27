#pragma once

#include <string>

namespace gridiron {

struct TeamProfile {
    std::string name;
    double offensive_pass_rating = 0.0;
    double offensive_run_rating = 0.0;
    double defensive_pass_rating = 0.0;
    double defensive_run_rating = 0.0;
    double turnover_rate = 0.0;
    double field_goal_rating = 0.0;
};

} // namespace gridiron
