#pragma once

#include <cstdint>

namespace eyesee {

/** Calibrated pulse widths (microseconds) for all six channels — the hardware-facing payload. */
struct ServoOutput {
    uint16_t lr = 0;
    uint16_t ud = 0;
    uint16_t tl = 0;
    uint16_t bl = 0;
    uint16_t tr = 0;
    uint16_t br = 0;
};

/**
 * Abstracts the physical actuator hardware. Deliberately not PCA9685-specific
 * — future implementations could target ESP32 LEDC PWM, a different PWM
 * chip, Dynamixel/CAN servos, or a host-side simulator for desktop testing.
 *
 * Owns no state beyond whatever the underlying driver chip itself holds
 * (see docs/architecture.md invariant 2).
 */
class IServoOutput {
public:
    virtual ~IServoOutput() = default;

    /** Primary per-frame call: write all six calibrated channels at once. */
    virtual void write(const ServoOutput& output) = 0;

    /** Manual/diagnostic/calibration use — not used by the per-frame pipeline. */
    virtual void moveServo(uint8_t channel, float angleDegrees) = 0;
    virtual void setAngle(uint8_t channel, float angleDegrees) = 0;
    virtual void setPulse(uint8_t channel, uint16_t pulseUs) = 0;

    virtual void update(uint32_t deltaMs) = 0;
};

}  // namespace eyesee
