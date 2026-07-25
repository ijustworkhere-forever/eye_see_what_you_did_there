#pragma once

#include <cstdint>

#include "CalibrationManager.h"
#include "IServoOutput.h"

namespace eyesee {

/** Named facial expressions. Pose blending per expression is future work (docs/ROADMAP.md v0.3). */
enum class Expression { Neutral, Happy, Curious, Sleepy, Angry, Surprised };

/** Command-layer gaze intent: where to look, and how (speed/blink/hold) — consumed by IAnimationEngine, not EyeController. */
struct GazeTarget {
    float x = 0.0f;
    float y = 0.0f;
    float speed = 1.0f;
    bool blinkOnArrival = false;
    bool hold = false;
};

/**
 * The complete instantaneous "desired shape" of both eyes — no timing, no
 * easing, just values. IAnimationEngine produces one of these per frame;
 * EyeController converts it to a ServoOutput.
 */
struct EyePose {
    float lookX = 0.0f;         // normalized -1..1
    float lookY = 0.0f;         // normalized -1..1
    float upperLeftLid = 1.0f;  // normalized 0 (closed) .. 1 (open)
    float lowerLeftLid = 1.0f;
    float upperRightLid = 1.0f;
    float lowerRightLid = 1.0f;
};

/**
 * Sole owner of eye motion state. Converts an EyePose into calibrated servo
 * pulses and writes them via IServoOutput. Owns no timed state — no delays,
 * no timers, no easing, no animation (see docs/architecture.md invariant 1).
 */
class EyeController {
public:
    EyeController(IServoOutput& output, CalibrationManager& calibration);

    /** The one true primitive: applies a pose immediately. */
    void applyPose(const EyePose& pose);
    EyePose currentPose() const;

    /** Convenience wrappers over applyPose(), each preserving prior pose fields they don't touch. */
    void look(float x, float y);
    void blink();
    void winkLeft();
    void winkRight();
    void sleep();
    void wake();
    void setExpression(Expression expression);
    void setIdle();

    /** Reserved; currently a no-op, since EyeController owns no timed state. */
    void update(uint32_t deltaMs);

private:
    IServoOutput& output_;
    CalibrationManager& calibration_;
    EyePose currentPose_;

    ServoOutput toServoOutput(const EyePose& pose) const;
};

}  // namespace eyesee
