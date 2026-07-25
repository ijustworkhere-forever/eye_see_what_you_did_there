#pragma once

#include <cstdint>

namespace eyesee {

/** Per-servo calibration, expressed entirely in pulse widths (not angles) so it's independent of
 * servo brand. */
struct ServoConfig {
    uint8_t channel = 0;
    uint16_t minPulseUs = 1000;
    uint16_t maxPulseUs = 2000;
    uint16_t neutralPulseUs = 1500;
    int16_t mechanicalOffset = 0;
    bool inverted = false;
    bool mirrored = false;
};

/** Calibration for all six servos plus the eye's overall look range. */
struct EyeConfig {
    ServoConfig lr;
    ServoConfig ud;
    ServoConfig tl;
    ServoConfig bl;
    ServoConfig tr;
    ServoConfig br;
    float lookRangeDegrees = 30.0f;
};

/** Placeholder network settings — no networking implemented this pass. */
struct NetworkConfig {
    const char* ssid = "";
    const char* password = "";
    uint16_t webServerPort = 80;
};

/** Placeholder behavior timing — no behavior logic implemented this pass. */
struct BehaviorConfig {
    uint32_t idleTimeoutMs = 15000;
};

constexpr ServoConfig makeDefaultServoConfig(uint8_t channel) {
    ServoConfig config;
    config.channel = channel;
    return config;
}

/** Channel assignment: 0=LR, 1=UD, 2=TL, 3=BL, 4=TR, 5=BR. */
constexpr EyeConfig makeDefaultEyeConfig() {
    EyeConfig config;
    config.lr = makeDefaultServoConfig(0);
    config.ud = makeDefaultServoConfig(1);
    config.tl = makeDefaultServoConfig(2);
    config.bl = makeDefaultServoConfig(3);
    config.tr = makeDefaultServoConfig(4);
    config.br = makeDefaultServoConfig(5);
    return config;
}

}  // namespace eyesee
