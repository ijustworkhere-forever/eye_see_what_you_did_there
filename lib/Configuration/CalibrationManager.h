#pragma once

#include "Configuration.h"
#include "IStorage.h"

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

    /** Loads every persisted key that exists, leaving any missing key at its
     * current value -- a first boot with an empty namespace is indistinguishable
     * from "nothing persisted yet," not an error. Returns true if at least one
     * key was found and loaded. */
    bool loadFromStorage(IStorage& storage);
    /** Writes every field of the current EyeConfig to storage. */
    void saveToStorage(IStorage& storage) const;

private:
    EyeConfig config_;
    ServoConfig& mutableServoConfig(EyeChannel channel);
};

}  // namespace eyesee
