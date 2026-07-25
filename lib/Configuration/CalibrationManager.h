#pragma once

#include "Configuration.h"

namespace eyesee {

/**
 * Selects one of the six calibrated servos. Deliberately capitalized `Lr`/`Ud`/...
 * rather than ALL-CAPS: `BR` is a macro in the Xtensa SDK's xtensa/config/specreg.h
 * (pulled in transitively by the ESP32 Arduino core), and an ALL-CAPS enumerator
 * would be macro-expanded depending on include order.
 */
enum class EyeChannel {
    Lr,
    Ud,
    Tl,
    Bl,
    Tr,
    Br
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
