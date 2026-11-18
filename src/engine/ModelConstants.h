#pragma once

namespace gridiron {
// Hand-tuned m4-v1 placeholders; M5 replaces these with fitted parameters.
constexpr double kRunBaseYards = 4.3;
constexpr double kPassBaseYards = 10.5;
constexpr double kRunRatingScale = 6.0;
constexpr double kPassRatingScale = 10.0;
constexpr double kRunYardsSd = 3.0;
constexpr double kPassYardsSd = 8.0;
constexpr double kPlayTimeMean = 33.0;
constexpr double kLongPlayTimeMean = 43.0;
constexpr double kPlayTimeSd = 5.0;
constexpr int kFourthDownGoForItMaxDistance = 2;
constexpr double kPuntNetMeanYards = 41.0;
constexpr double kPuntNetSdYards = 8.0;
constexpr int kPuntTouchbackSpot = 20;
constexpr int kKickoffSpot = 35;
constexpr int kTurnoverTouchbackSpot = 20;
constexpr int kPuntSeconds = 6;
constexpr int kSnapSeconds = 6;
constexpr int kTwoMinuteWarningSeconds = 120;
constexpr int kLateQ4OutOfBoundsSeconds = 300;
constexpr double kOutOfBoundsRate = 0.2;
constexpr double kIncompleteRate = 0.35;
constexpr int kOvertimeSeconds = 600;
constexpr int kOvertimeTimeouts = 2;
constexpr int kTimeoutWindowSeconds = 120;
} // namespace gridiron
