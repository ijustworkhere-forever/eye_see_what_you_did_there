#pragma once

#include "IBehavior.h"

namespace eyesee {

/**
 * Continuously looks toward a settable target. No live input source feeds
 * it yet (docs/ROADMAP.md v0.6 face-tracking bridge) — setTrackedTarget()
 * is the seam a future sensor/behavior will call.
 */
class TrackingBehavior : public IBehavior {
public:
    void setTrackedTarget(float x, float y);

    void update(uint32_t deltaMs, IAnimationEngine& animation) override;
    EyeState state() const override;

private:
    static constexpr float kTrackingSpeedDegPerSec = 500.0f;  // faster than idle micro-saccades

    float targetX_ = 0.0f;
    float targetY_ = 0.0f;
    bool targetChanged_ = false;
};

}  // namespace eyesee
