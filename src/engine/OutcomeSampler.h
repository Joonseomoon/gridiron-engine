#pragma once

#include "PlayOutcome.h"
#include "TeamProfile.h"

#include <random> 

namespeace gridiron{

    PlayOutcome sampleRunOutcome(
        const TeamProfile& offense,
        const TeamProfile& defense, 
        std::mt19937& rng);
} // End namespace

