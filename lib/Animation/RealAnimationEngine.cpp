#include "RealAnimationEngine.h"

#include <algorithm>
#include <cmath>

namespace eyesee {

namespace {
float lerp(float start, float target, float t) {
    return start + (target - start) * t;
}
}  // namespace

RealAnimationEngine::RealAnimationEngine(EyeController& eyeController,
                                         CalibrationManager& calibration)
    : eyeController_(eyeController), calibration_(calibration) {
    const EyePose initial = eyeController_.currentPose();
    gaze_.currentX = gaze_.startX = gaze_.targetX = initial.lookX;
    gaze_.currentY = gaze_.startY = gaze_.targetY = initial.lookY;
    eyelid_.currentUpperLeftLid = eyelid_.startUpperLeftLid = eyelid_.targetUpperLeftLid =
        initial.upperLeftLid;
    eyelid_.currentLowerLeftLid = eyelid_.startLowerLeftLid = eyelid_.targetLowerLeftLid =
        initial.lowerLeftLid;
    eyelid_.currentUpperRightLid = eyelid_.startUpperRightLid = eyelid_.targetUpperRightLid =
        initial.upperRightLid;
    eyelid_.currentLowerRightLid = eyelid_.startLowerRightLid = eyelid_.targetLowerRightLid =
        initial.lowerRightLid;
}

void RealAnimationEngine::animateGaze(const GazeTarget& target) {
    const float deltaX = std::fabs(target.x - gaze_.currentX);
    const float deltaY = std::fabs(target.y - gaze_.currentY);
    const float deltaNormalized = std::max(deltaX, deltaY);
    uint32_t durationMs = 0;
    if (target.speed > 0.0f) {
        const float lookRangeDegrees = calibration_.eyeConfig().lookRangeDegrees;
        // Clamp to a small positive floor so a pathologically tiny speed can't blow up the
        // division, and round (rather than truncate toward zero) so the result matches the
        // real-valued duration instead of always underestimating it.
        const float safeSpeed = std::max(target.speed, 0.001f);
        durationMs = static_cast<uint32_t>(
            std::lround((deltaNormalized * lookRangeDegrees / safeSpeed) * 1000.0f));
        durationMs = std::min(durationMs, kMaxGazeDurationMs);
    }
    startGazeTransition(target.x, target.y, durationMs, target.blinkOnArrival);
}

void RealAnimationEngine::animateBlink(uint32_t durationMs) {
    startEyelidTransition(0.0f, 0.0f, 0.0f, 0.0f, durationMs);
    eyelid_.autoReopenOnComplete = true;
}

void RealAnimationEngine::animateWinkLeft(uint32_t durationMs) {
    startEyelidTransition(0.0f, 0.0f, eyelid_.currentUpperRightLid, eyelid_.currentLowerRightLid,
                          durationMs);
}

void RealAnimationEngine::animateWinkRight(uint32_t durationMs) {
    startEyelidTransition(eyelid_.currentUpperLeftLid, eyelid_.currentLowerLeftLid, 0.0f, 0.0f,
                          durationMs);
}

void RealAnimationEngine::animateSleep(uint32_t durationMs) {
    startEyelidTransition(0.0f, 0.0f, 0.0f, 0.0f, durationMs);
}

void RealAnimationEngine::animateWake(uint32_t durationMs) {
    startEyelidTransition(1.0f, 1.0f, 1.0f, 1.0f, durationMs);
}

void RealAnimationEngine::animateExpression(Expression expression, uint32_t durationMs) {
    (void)durationMs;
    // TODO: per-expression target pose blending (docs/ROADMAP.md v0.3). No pose
    // change this pass — matches EyeController::setExpression()'s no-op.
    eyeController_.setExpression(expression);
}

void RealAnimationEngine::update(uint32_t deltaMs) {
    if (!gaze_.active && !eyelid_.active) {
        return;
    }

    bool gazeJustCompletedWithBlink = false;
    bool eyelidJustCompletedWithReopen = false;
    uint32_t reopenDurationMs = 0;

    if (gaze_.active) {
        gaze_.elapsedMs += deltaMs;
        const float rawT = gaze_.durationMs == 0
                               ? 1.0f
                               : std::min(1.0f, static_cast<float>(gaze_.elapsedMs) /
                                                    static_cast<float>(gaze_.durationMs));
        const float t = ease(EasingType::Cubic, rawT);
        gaze_.currentX = lerp(gaze_.startX, gaze_.targetX, t);
        gaze_.currentY = lerp(gaze_.startY, gaze_.targetY, t);
        if (gaze_.elapsedMs >= gaze_.durationMs) {
            gaze_.currentX = gaze_.targetX;
            gaze_.currentY = gaze_.targetY;
            gaze_.active = false;
            gazeJustCompletedWithBlink = gaze_.blinkOnArrival;
            gaze_.blinkOnArrival = false;
        }
    }

    if (eyelid_.active) {
        eyelid_.elapsedMs += deltaMs;
        const float rawT = eyelid_.durationMs == 0
                               ? 1.0f
                               : std::min(1.0f, static_cast<float>(eyelid_.elapsedMs) /
                                                    static_cast<float>(eyelid_.durationMs));
        const float t = ease(EasingType::EaseInOut, rawT);
        eyelid_.currentUpperLeftLid =
            lerp(eyelid_.startUpperLeftLid, eyelid_.targetUpperLeftLid, t);
        eyelid_.currentLowerLeftLid =
            lerp(eyelid_.startLowerLeftLid, eyelid_.targetLowerLeftLid, t);
        eyelid_.currentUpperRightLid =
            lerp(eyelid_.startUpperRightLid, eyelid_.targetUpperRightLid, t);
        eyelid_.currentLowerRightLid =
            lerp(eyelid_.startLowerRightLid, eyelid_.targetLowerRightLid, t);
        if (eyelid_.elapsedMs >= eyelid_.durationMs) {
            eyelid_.currentUpperLeftLid = eyelid_.targetUpperLeftLid;
            eyelid_.currentLowerLeftLid = eyelid_.targetLowerLeftLid;
            eyelid_.currentUpperRightLid = eyelid_.targetUpperRightLid;
            eyelid_.currentLowerRightLid = eyelid_.targetLowerRightLid;
            eyelid_.active = false;
            eyelidJustCompletedWithReopen = eyelid_.autoReopenOnComplete;
            reopenDurationMs = eyelid_.durationMs;
            eyelid_.autoReopenOnComplete = false;
        }
    }

    EyePose pose;
    pose.lookX = gaze_.currentX;
    pose.lookY = gaze_.currentY;
    pose.upperLeftLid = eyelid_.currentUpperLeftLid;
    pose.lowerLeftLid = eyelid_.currentLowerLeftLid;
    pose.upperRightLid = eyelid_.currentUpperRightLid;
    pose.lowerRightLid = eyelid_.currentLowerRightLid;
    eyeController_.applyPose(pose);

    if (gazeJustCompletedWithBlink) {
        animateBlink(kDefaultBlinkDurationMs);
    }
    if (eyelidJustCompletedWithReopen) {
        animateWake(reopenDurationMs);
    }
}

void RealAnimationEngine::startGazeTransition(float targetX, float targetY, uint32_t durationMs,
                                              bool blinkOnArrival) {
    gaze_.startX = gaze_.currentX;
    gaze_.startY = gaze_.currentY;
    gaze_.targetX = targetX;
    gaze_.targetY = targetY;
    gaze_.elapsedMs = 0;
    gaze_.durationMs = durationMs;
    gaze_.active = true;
    gaze_.blinkOnArrival = blinkOnArrival;
}

void RealAnimationEngine::startEyelidTransition(float targetUpperLeftLid, float targetLowerLeftLid,
                                                float targetUpperRightLid,
                                                float targetLowerRightLid, uint32_t durationMs) {
    eyelid_.startUpperLeftLid = eyelid_.currentUpperLeftLid;
    eyelid_.startLowerLeftLid = eyelid_.currentLowerLeftLid;
    eyelid_.startUpperRightLid = eyelid_.currentUpperRightLid;
    eyelid_.startLowerRightLid = eyelid_.currentLowerRightLid;
    eyelid_.targetUpperLeftLid = targetUpperLeftLid;
    eyelid_.targetLowerLeftLid = targetLowerLeftLid;
    eyelid_.targetUpperRightLid = targetUpperRightLid;
    eyelid_.targetLowerRightLid = targetLowerRightLid;
    eyelid_.elapsedMs = 0;
    eyelid_.durationMs = durationMs;
    eyelid_.active = true;
    eyelid_.autoReopenOnComplete = false;
}

}  // namespace eyesee
