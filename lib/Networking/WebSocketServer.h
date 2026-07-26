#pragma once

#include <cstdint>

#include <ESPAsyncWebServer.h>

#include "EyeController.h"
#include "IBehaviorEngine.h"

namespace eyesee {

/** Broadcasts the current EyeState/EyePose to every connected WebSocket client at
 * ~30Hz (docs/superpowers/specs/2026-07-25-v0.4-connectivity-design.md). */
class WebSocketServer {
public:
    WebSocketServer(const IBehaviorEngine& behaviorEngine, const EyeController& eyeController);

    /** Mounts the WebSocket handler at /ws. Call once from setup(), before server.begin(). */
    void begin(AsyncWebServer& server);
    /** Rate-limits and broadcasts; also periodically cleans up disconnected clients. */
    void update(uint32_t deltaMs);

private:
    const IBehaviorEngine& behaviorEngine_;
    const EyeController& eyeController_;
    AsyncWebSocket ws_{"/ws"};
    uint32_t msSinceLastBroadcast_ = 0;
    static constexpr uint32_t kBroadcastPeriodMs = 33;  // ~30Hz, low end of the documented 30-60Hz range
};

}  // namespace eyesee
