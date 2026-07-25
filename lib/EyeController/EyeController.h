#pragma once

#include <cstdint>

#include "CalibrationManager.h"
#include "EyeTypes.h"
#include "IServoOutput.h"

namespace eyesee {

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

    /** Convenience wrappers over applyPose(), each preserving prior pose fields they don't touch.
     */
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
