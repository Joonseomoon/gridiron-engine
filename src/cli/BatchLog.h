#pragma once

#include "../engine/BatchSummary.h"
#include <ostream>
#include <string>
#include <vector>

namespace gridiron {

void printBatchSummary(std::ostream& out, const std::string& homeName, const std::string& awayName,
                       const BatchSummary& summary);

void printConvergence(std::ostream& out, const std::string& homeName, const std::vector<ConvergencePoint>& points);
} // namespace gridiron
