#pragma once

#include <string>

namespace gridiron {

struct TeamProfile {
    std::string name;
    double offensive_pass_rating;
    double offensive_run_rating;
    double defensive_pass_rating;
    double defensive_run_rating;
    double turnover_rate;
    double field_goal_rating;
};

} // namespace gridiron
