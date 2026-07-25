#pragma once

#include "IServoOutput.h"

// This class is Arduino-only, and so is its entire implementation in
// Pca9685ServoOutput.cpp. The declaration is guarded to match: under !ARDUINO
// this header intentionally declares nothing, so a native build can't reference
// a class that has no definitions anywhere. Native tests use a fake
// IServoOutput instead (see test/test_native/support/FakeServoOutput.h).
#ifdef ARDUINO

#include <Adafruit_PWMServoDriver.h>

namespace eyesee {

/** IServoOutput implementation driving a PCA9685 over I2C. */
class Pca9685ServoOutput : public IServoOutput {
public:
    explicit Pca9685ServoOutput(uint8_t i2cAddress = 0x40);

    /** Starts I2C and configures the PWM frequency. Call once from setup(). */
    void init();

    void write(const ServoOutput& output) override;
    void moveServo(uint8_t channel, float angleDegrees) override;
    void setAngle(uint8_t channel, float angleDegrees) override;
    void setPulse(uint8_t channel, uint16_t pulseUs) override;
    void update(uint32_t deltaMs) override;

private:
    Adafruit_PWMServoDriver driver_;

    static constexpr uint8_t kChannelLr = 0;
    static constexpr uint8_t kChannelUd = 1;
    static constexpr uint8_t kChannelTl = 2;
    static constexpr uint8_t kChannelBl = 3;
    static constexpr uint8_t kChannelTr = 4;
    static constexpr uint8_t kChannelBr = 5;
    static constexpr uint16_t kPwmFrequencyHz = 50;
};

}  // namespace eyesee

#endif  // ARDUINO
