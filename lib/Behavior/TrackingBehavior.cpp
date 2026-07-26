#include "TrackingBehavior.h"

#include "IAnimationEngine.h"

namespace eyesee {

void TrackingBehavior::setTrackedTarget(float x, float y) {
    targetX_ = x;
    targetY_ = y;
    targetChanged_ = true;
}

void TrackingBehavior::update(uint32_t deltaMs, IAnimationEngine& animation) {
    (void)deltaMs;
    // Only issue a new animateGaze() when the target actually changed — calling it
    // every frame would restart the eased transition from scratch each tick and the
    // eye would never actually finish arriving anywhere.
    if (!targetChanged_) {
        return;
    }
    targetChanged_ = false;

    GazeTarget target;
    target.x = targetX_;
    target.y = targetY_;
    target.speed = kTrackingSpeedDegPerSec;
    target.hold = true;
    animation.animateGaze(target);
}

EyeState TrackingBehavior::state() const {
    return EyeState::Tracking;
}

}  // namespace eyesee
