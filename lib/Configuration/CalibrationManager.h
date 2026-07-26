#pragma once

#include "Configuration.h"
#include "IStorage.h"

#ifdef ARDUINO
#include <freertos/FreeRTOS.h>
#endif

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
 * Persistence to flash is implemented via `loadFromStorage`/`saveToStorage`
 * below (see `docs/superpowers/specs/2026-07-26-v0.5-persistence-ota-design.md`).
 * `setServoConfig()` itself stays storage-agnostic; callers (currently `RestApi`'s
 * config route) explicitly call `saveToStorage()` after mutating.
 *
 * Thread-safe on ESP32: every public method is guarded by a FreeRTOS critical
 * section, since `RestApi`'s `POST /api/v1/config` route (which calls
 * `setServoConfig()`) runs on AsyncTCP's own task while `EyeController` reads
 * `eyeConfig()` every frame from the Arduino `loop()` task -- the same
 * cross-task hazard `CommandQueue` already guards against (see
 * `lib/Behavior/CommandQueue.h`). `eyeConfig()`/`servoConfig()` return by value
 * (a locked snapshot copy), not by reference, so the lock actually protects the
 * data the caller reads, not just the accessor call itself. Native tests are
 * single-threaded, so the guard compiles away to nothing there.
 */
class CalibrationManager {
public:
    CalibrationManager();
    explicit CalibrationManager(const EyeConfig& initialConfig);

    EyeConfig eyeConfig() const;
    ServoConfig servoConfig(EyeChannel channel) const;
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
    const ServoConfig& constServoConfig(EyeChannel channel) const;
#ifdef ARDUINO
    mutable portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
#endif
};

}  // namespace eyesee
