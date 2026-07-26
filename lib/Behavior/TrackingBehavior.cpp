#include "TrackingBehavior.h"

#include "IAnimationEngine.h"

namespace eyesee {

void TrackingBehavior::receiveGazeTarget(const GazeTarget& target) {
    lastTarget_ = target;
    targetChanged_ = true;
    hasTarget_ = true;
    recentered_ = false;
    msSinceLastUpdate_ = 0;
}

void TrackingBehavior::update(uint32_t deltaMs, IAnimationEngine& animation) {
    if (targetChanged_) {
        targetChanged_ = false;
        GazeTarget target = lastTarget_;
        target.speed = kTrackingSpeedDegPerSec;
        animation.animateGaze(target);
        return;
    }

    if (!hasTarget_ || recentered_) {
        return;
    }

    msSinceLastUpdate_ += deltaMs;
    if (msSinceLastUpdate_ < kStaleTimeoutMs) {
        return;
    }

    if (lastTarget_.hold) {
        return;  // hold indefinitely -- do not recenter, do not re-check every frame past the
                 // timeout
    }

    recentered_ = true;
    GazeTarget center;
    center.x = 0.0f;
    center.y = 0.0f;
    center.speed = kTrackingSpeedDegPerSec;
    animation.animateGaze(center);
}

EyeState TrackingBehavior::state() const {
    return EyeState::Tracking;
}

}  // namespace eyesee
