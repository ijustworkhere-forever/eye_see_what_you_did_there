#pragma once

#include "IServoOutput.h"

#ifdef ARDUINO
#include <Adafruit_PWMServoDriver.h>
#else
// Forward declaration for non-Arduino environments
class Adafruit_PWMServoDriver;
#endif

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
#ifdef ARDUINO
    Adafruit_PWMServoDriver driver_;
#else
    void* driver_;  // Placeholder for non-Arduino environments
#endif

    static constexpr uint8_t kChannelLr = 0;
    static constexpr uint8_t kChannelUd = 1;
    static constexpr uint8_t kChannelTl = 2;
    static constexpr uint8_t kChannelBl = 3;
    static constexpr uint8_t kChannelTr = 4;
    static constexpr uint8_t kChannelBr = 5;
    static constexpr uint16_t kPwmFrequencyHz = 50;
};

}  // namespace eyesee
