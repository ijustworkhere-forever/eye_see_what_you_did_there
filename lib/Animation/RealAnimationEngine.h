#pragma once

#include <cstdint>

#include "CalibrationManager.h"
#include "Easing.h"
#include "EyeController.h"
#include "IAnimationEngine.h"

namespace eyesee {

/**
 * Real interpolation: an independent gaze transition and eyelid transition
 * are each eased over their own duration and composed into one EyePose per
 * frame. See docs/superpowers/specs/2026-07-25-v0.2-real-motion-design.md.
 *
 * `gaze_`/`eyelid_` cache the full pose from construction onward and are
 * the sole intended writer to `EyeController` for as long as this engine is
 * in use — any direct `EyeController` calls (`look()`, `blink()`, etc.) made
 * elsewhere while this engine is active will be silently overwritten on the
 * next `update()`.
 */
class RealAnimationEngine : public IAnimationEngine {
public:
    RealAnimationEngine(EyeController& eyeController, CalibrationManager& calibration);

    void animateGaze(const GazeTarget& target) override;
    void animateBlink(uint32_t durationMs) override;
    void animateWinkLeft(uint32_t durationMs) override;
    void animateWinkRight(uint32_t durationMs) override;
    void animateSleep(uint32_t durationMs) override;
    void animateWake(uint32_t durationMs) override;
    void animateExpression(Expression expression, uint32_t durationMs) override;
    void update(uint32_t deltaMs) override;

private:
    static constexpr uint32_t kDefaultBlinkDurationMs = 150;

    struct GazeTransition {
        float currentX = 0.0f, currentY = 0.0f;
        float startX = 0.0f, startY = 0.0f;
        float targetX = 0.0f, targetY = 0.0f;
        uint32_t elapsedMs = 0;
        uint32_t durationMs = 0;
        bool active = false;
        bool blinkOnArrival = false;
    };

    struct EyelidTransition {
        float currentUpperLeftLid = 1.0f, currentLowerLeftLid = 1.0f;
        float currentUpperRightLid = 1.0f, currentLowerRightLid = 1.0f;
        float startUpperLeftLid = 1.0f, startLowerLeftLid = 1.0f;
        float startUpperRightLid = 1.0f, startLowerRightLid = 1.0f;
        float targetUpperLeftLid = 1.0f, targetLowerLeftLid = 1.0f;
        float targetUpperRightLid = 1.0f, targetLowerRightLid = 1.0f;
        uint32_t elapsedMs = 0;
        uint32_t durationMs = 0;
        bool active = false;
    };

    EyeController& eyeController_;
    CalibrationManager& calibration_;
    GazeTransition gaze_;
    EyelidTransition eyelid_;

    void startGazeTransition(float targetX, float targetY, uint32_t durationMs, bool blinkOnArrival);
    void startEyelidTransition(float targetUpperLeftLid, float targetLowerLeftLid,
                               float targetUpperRightLid, float targetLowerRightLid,
                               uint32_t durationMs);
};

}  // namespace eyesee
