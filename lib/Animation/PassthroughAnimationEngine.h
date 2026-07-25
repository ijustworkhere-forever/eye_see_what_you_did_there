#pragma once

#include "EyeController.h"
#include "IAnimationEngine.h"

namespace eyesee {

/**
 * IAnimationEngine implementation with no real interpolation — every
 * animate*() call applies its target pose immediately via EyeController.
 * The seam is real even though the easing/timing math isn't (docs/ROADMAP.md v0.2).
 */
class PassthroughAnimationEngine : public IAnimationEngine {
public:
    explicit PassthroughAnimationEngine(EyeController& eyeController);

    void animateGaze(const GazeTarget& target) override;
    void animateBlink(uint32_t durationMs) override;
    void animateWinkLeft(uint32_t durationMs) override;
    void animateWinkRight(uint32_t durationMs) override;
    void animateSleep(uint32_t durationMs) override;
    void animateWake(uint32_t durationMs) override;
    void animateExpression(Expression expression, uint32_t durationMs) override;
    void update(uint32_t deltaMs) override;

private:
    EyeController& eyeController_;
};

}  // namespace eyesee
