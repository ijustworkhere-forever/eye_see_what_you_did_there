#include "Pca9685ServoOutput.h"

#ifdef ARDUINO

namespace eyesee {

Pca9685ServoOutput::Pca9685ServoOutput(uint8_t i2cAddress) : driver_(i2cAddress) {
}

void Pca9685ServoOutput::init() {
    driver_.begin();
    driver_.setPWMFreq(kPwmFrequencyHz);
}

void Pca9685ServoOutput::write(const ServoOutput& output) {
    setPulse(kChannelLr, output.lr);
    setPulse(kChannelUd, output.ud);
    setPulse(kChannelTl, output.tl);
    setPulse(kChannelBl, output.bl);
    setPulse(kChannelTr, output.tr);
    setPulse(kChannelBr, output.br);
}

void Pca9685ServoOutput::moveServo(uint8_t channel, float angleDegrees) {
    setAngle(channel, angleDegrees);
}

void Pca9685ServoOutput::setAngle(uint8_t channel, float angleDegrees) {
    constexpr float kMinPulseUs = 1000.0f;
    constexpr float kMaxPulseUs = 2000.0f;
    constexpr float kMaxAngleDegrees = 180.0f;

    const float clamped = angleDegrees < 0.0f
                              ? 0.0f
                              : (angleDegrees > kMaxAngleDegrees ? kMaxAngleDegrees : angleDegrees);
    const float pulseUs = kMinPulseUs + (clamped / kMaxAngleDegrees) * (kMaxPulseUs - kMinPulseUs);
    setPulse(channel, static_cast<uint16_t>(pulseUs));
}

void Pca9685ServoOutput::setPulse(uint8_t channel, uint16_t pulseUs) {
    constexpr uint32_t kTicksPerCycle = 4096;
    constexpr uint32_t kMaxTicks = kTicksPerCycle - 1;  // 12-bit PWM counter: 0..4095
    const uint32_t microsPerCycle = 1000000UL / kPwmFrequencyHz;
    const uint32_t ticks = (static_cast<uint32_t>(pulseUs) * kTicksPerCycle) / microsPerCycle;
    // Clamp before narrowing: a pulseUs longer than one PWM cycle yields
    // ticks > 4095, which would truncate into a garbage duty cycle.
    const uint32_t clampedTicks = ticks > kMaxTicks ? kMaxTicks : ticks;
    driver_.setPWM(channel, 0, static_cast<uint16_t>(clampedTicks));
}

void Pca9685ServoOutput::update(uint32_t deltaMs) {
    (void)deltaMs;
    // TODO: reserved (see docs/architecture.md invariant 2 — MotionHardware owns no state).
}

}  // namespace eyesee

#endif  // ARDUINO
