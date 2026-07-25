#pragma once

#include "Configuration.h"

// BR collides with a macro defined in xtensa/config/specreg.h (pulled in transitively
// via the ESP32 Arduino core). This undef prevents macro expansion in the enum definition.
#undef BR

namespace eyesee {

/** Selects one of the six calibrated servos. */
enum class EyeChannel {
    LR,
    UD,
    TL,
    BL,
    TR,
    BR
};

/**
 * Owns the in-memory EyeConfig: per-servo limits, neutral position,
 * inversion, mirroring, and mechanical offset.
 *
 * Persistence to flash (via IStorage) is not implemented this pass —
 * see docs/ROADMAP.md v0.5.
 */
class CalibrationManager {
public:
    CalibrationManager();
    explicit CalibrationManager(const EyeConfig& initialConfig);

    const EyeConfig& eyeConfig() const;
    const ServoConfig& servoConfig(EyeChannel channel) const;
    void setServoConfig(EyeChannel channel, const ServoConfig& config);

private:
    EyeConfig config_;
    ServoConfig& mutableServoConfig(EyeChannel channel);
};

}  // namespace eyesee
