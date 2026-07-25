#pragma once

#include "Configuration.h"

// Undefine macros that may conflict with enum values
#undef LR
#undef UD
#undef TL
#undef BL
#undef TR
#undef BR

namespace eyesee {

/** Selects one of the six calibrated servos. */
enum class EyeChannel { LR, UD, TL, BL, TR, BR };

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
