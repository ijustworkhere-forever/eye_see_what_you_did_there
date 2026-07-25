#pragma once

#include <cstdint>

// Only GazeTarget/Expression are needed for these signatures — the concrete
// EyeController class is an implementation detail of the concrete engines.
#include "EyeTypes.h"

namespace eyesee {

/**
 * Converts command-layer intent (GazeTarget/Expression + duration) into a
 * sequence of EyePose updates over time. This is where "350ms, cubic
 * easing" lives — EyeController never sees timing.
 */
class IAnimationEngine {
public:
    virtual ~IAnimationEngine() = default;

    virtual void animateGaze(const GazeTarget& target) = 0;
    virtual void animateBlink(uint32_t durationMs) = 0;
    virtual void animateWinkLeft(uint32_t durationMs) = 0;
    virtual void animateWinkRight(uint32_t durationMs) = 0;
    virtual void animateSleep(uint32_t durationMs) = 0;
    virtual void animateWake(uint32_t durationMs) = 0;
    virtual void animateExpression(Expression expression, uint32_t durationMs) = 0;

    /** Advances any in-progress animation; calls EyeController::applyPose() as needed. */
    virtual void update(uint32_t deltaMs) = 0;
};

}  // namespace eyesee
