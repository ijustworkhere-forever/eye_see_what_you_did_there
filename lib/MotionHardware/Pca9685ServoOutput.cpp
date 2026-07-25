#include "Pca9685ServoOutput.h"

#ifdef ARDUINO
#include <Adafruit_PWMServoDriver.h>
#endif

namespace eyesee {

#ifdef ARDUINO
Pca9685ServoOutput::Pca9685ServoOutput(uint8_t i2cAddress) : driver_(i2cAddress) {}

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
    const uint32_t microsPerCycle = 1000000UL / kPwmFrequencyHz;
    const uint32_t ticks = (static_cast<uint32_t>(pulseUs) * kTicksPerCycle) / microsPerCycle;
    driver_.setPWM(channel, 0, static_cast<uint16_t>(ticks));
}

void Pca9685ServoOutput::update(uint32_t deltaMs) {
    (void)deltaMs;
    // TODO: reserved (see docs/architecture.md invariant 2 — MotionHardware owns no state).
}

#else
// Stub implementations for non-Arduino environments
Pca9685ServoOutput::Pca9685ServoOutput(uint8_t i2cAddress) : driver_(nullptr) {
    (void)i2cAddress;
}

void Pca9685ServoOutput::init() {}
void Pca9685ServoOutput::write(const ServoOutput& output) { (void)output; }
void Pca9685ServoOutput::moveServo(uint8_t channel, float angleDegrees) { (void)channel; (void)angleDegrees; }
void Pca9685ServoOutput::setAngle(uint8_t channel, float angleDegrees) { (void)channel; (void)angleDegrees; }
void Pca9685ServoOutput::setPulse(uint8_t channel, uint16_t pulseUs) { (void)channel; (void)pulseUs; }
void Pca9685ServoOutput::update(uint32_t deltaMs) { (void)deltaMs; }
#endif

}  // namespace eyesee
