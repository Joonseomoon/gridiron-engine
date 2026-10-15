#pragma once

#include "../engine/BatchMetadata.h"
#include "../engine/BatchSummary.h"
#include "../engine/SimulationResult.h"

#include <ostream>
#include <vector>

namespace gridiron {

void writeBatchCsv(std::ostream& out, const BatchMetadata& metadata, const std::vector<SimulationResult>& results,
                   const BatchSummary& summary);
} // namespace gridiron
