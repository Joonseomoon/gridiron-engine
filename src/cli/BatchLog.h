#pragma once

#include "../engine/BatchSummary.h"
#include <ostream>
#include <string>

namespace gridiron {

void printBatchSummary(std::ostream& out, const std::string& homeName, const std::string& awayName,
                       const BatchSummary& summary);
}