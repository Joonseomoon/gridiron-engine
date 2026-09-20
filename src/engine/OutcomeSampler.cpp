#include "OutcomeSampler.h"

#include <random>
#include <cmath>
#include <algorithm> 

namespace gridiron{
    
    PlayOutcome samplePlayOutcome(const TeamProfile& offense, const TeamProfile& defense, std::mt19937& rng){
        PlayType playType = chosePlayType(rng);

        if (playType == PlayType::Run){
            return sampleRunOutcome(offense, defense, rng);
        }
        return samplePassOutcome(offense, defense, rng);
    }

    PlayType chosePlayType(std::mt19937& rng){
        std::uniform_real_distribution<double> passOrRunDistribution(0.0, 1.0);
        if (passOrRunDistribution(rng) > 0.5){
            return PlayType::Run;
        }
        return PlayType::Pass;
    }

    PlayOutcome sampleRunOutcome(const TeamProfile& offense, const TeamProfile& defense, std::mt19937& rng){
        const int baseYards {2}; 
        const double expectedYards {baseYards + 10*(offense.offensive_run_rating - defense.defensive_run_rating)};
        std::normal_distribution<double> playYardsDistribution (expectedYards, 3.0);
        int playYards = static_cast<int>(std::round(playYardsDistribution(rng)));
        int playTime {};

        if (playYards > 20){
            std::normal_distribution<double> largePlayTimeDistribtion (30, 5);
            playTime = std::max(15, static_cast<int>(std::round(largePlayTimeDistribtion(rng))));
        } 
        else{
            std::normal_distribution<double> smallPlayTimeDistribtion (20, 5);
            playTime = std::max(10, static_cast<int>(std::round(smallPlayTimeDistribtion(rng))));
        }

        std::uniform_real_distribution<double> turnoverDistribution (0.0, 1.0);
        bool turnover = turnoverDistribution(rng) < offense.turnover_rate;

        if(turnover){
            return PlayOutcome{
                PlayType::Run,
                OutcomeType::Turnover,
                playYards, 
                playTime
            };
        }
        return PlayOutcome{
                PlayType::Run,
                OutcomeType::Normal,
                playYards, 
                playTime
            };

    }

    PlayOutcome samplePassOutcome(const TeamProfile& offense, const TeamProfile& defense, std::mt19937& rng){
        const int baseYards {4}; 
        const double expectedYards {baseYards + 15*(offense.offensive_pass_rating - defense.defensive_pass_rating)};
        std::normal_distribution<double> playYardsDistribution (expectedYards, 5.0);
        int playYards = static_cast<int>(std::round(playYardsDistribution(rng)));
        int playTime {};

        if (playYards > 20){
            std::normal_distribution<double> largePlayTimeDistribtion (30, 5);
            playTime = std::max(15, static_cast<int>(std::round(largePlayTimeDistribtion(rng))));
        } 
        else{
            std::normal_distribution<double> smallPlayTimeDistribtion (20, 5);
            playTime = std::max(10, static_cast<int>(std::round(smallPlayTimeDistribtion(rng))));
        }

        std::uniform_real_distribution<double> turnoverDistribution (0.0, 1.0);
        bool turnover = turnoverDistribution(rng) < offense.turnover_rate;

        if(turnover){
            return PlayOutcome{
                PlayType::Pass,
                OutcomeType::Turnover,
                playYards, 
                playTime
            };
        }
        return PlayOutcome{
                PlayType::Pass,
                OutcomeType::Normal,
                playYards, 
                playTime
            };

    }
}