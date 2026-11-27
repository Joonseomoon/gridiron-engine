#pragma once

#include "engine/batch/BatchMetadata.h"
#include "engine/batch/BatchSummary.h"
#include "engine/batch/SimulationResult.h"

#include <ostream>
#include <vector>

namespace gridiron {

void writeBatchCsv(std::ostream& out, const BatchMetadata& metadata, const std::vector<SimulationResult>& results,
                   const BatchSummary& summary);
} // namespace gridiron
