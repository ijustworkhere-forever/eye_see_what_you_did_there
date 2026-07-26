#pragma once

#include <cstdint>

#include "CalibrationManager.h"
#include "CommandQueue.h"
#include "EyeController.h"
#include "IBehaviorEngine.h"
#include "IStorage.h"
#include "WifiManager.h"

class AsyncWebServer;

namespace eyesee {

/**
 * Registers /api/v1/* routes on the shared AsyncWebServer. Every route
 * either reads existing read-only state (status) or pushes an EyeCommand
 * into CommandQueue (everything else) -- never touches IAnimationEngine or
 * hardware directly.
 */
class RestApi {
public:
    RestApi(CommandQueue& commandQueue, const IBehaviorEngine& behaviorEngine,
            const EyeController& eyeController, const WifiManager& wifiManager,
            CalibrationManager& calibrationManager, IStorage& storage);

    /** Registers all routes. Call once from setup(), before server.begin(). */
    void begin(AsyncWebServer& server);
    /** No-op -- request-driven; kept for the shared per-frame update() convention. */
    void update(uint32_t deltaMs);

private:
    CommandQueue& commandQueue_;
    const IBehaviorEngine& behaviorEngine_;
    const EyeController& eyeController_;
    const WifiManager& wifiManager_;
    CalibrationManager& calibrationManager_;
    IStorage& storage_;
};

}  // namespace eyesee
