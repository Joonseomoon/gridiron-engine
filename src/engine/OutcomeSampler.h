#pragma once

#include "PlayOutcome.h"
#include "TeamProfile.h"

#include <random> 

namespace gridiron{

    PlayOutcome samplePlayOutcome(
        const TeamProfile& offense,
        const TeamProfile& defense, 
        std::mt19937& rng);

    PlayType playOutcome(std::mt19937& rng);

    PlayOutcome sampleRunOutcome(
        const TeamProfile& offense,
        const TeamProfile& defense, 
        std::mt19937& rng);

    
    PlayOutcome samplePassOutcome(
        const TeamProfile& offense,
        const TeamProfile& defense, 
        std::mt19937& rng);
} // End namespace

