#pragma once

#include "IBehavior.h"

namespace eyesee {

/**
 * Continuously looks toward a target delivered via receiveGazeTarget()
 * (fed by BehaviorEngine::dispatch()'s CommandType::Track case -- see
 * docs/superpowers/specs/2026-07-26-v0.6-integrations-design.md).
 * If no new target arrives for kStaleTimeoutMs, recenters once (unless the
 * last-received target's `hold` was true, in which case it holds the last
 * position indefinitely).
 */
class TrackingBehavior : public IBehavior {
public:
    void receiveGazeTarget(const GazeTarget& target) override;

    void update(uint32_t deltaMs, IAnimationEngine& animation) override;
    EyeState state() const override;

private:
    static constexpr float kTrackingSpeedDegPerSec = 500.0f;  // faster than idle micro-saccades
    static constexpr uint32_t kStaleTimeoutMs = 3000;         // no update in 3s -> face considered lost

    GazeTarget lastTarget_{};
    bool targetChanged_ = false;
    bool hasTarget_ = false;
    bool recentered_ = false;
    uint32_t msSinceLastUpdate_ = 0;
};

}  // namespace eyesee
