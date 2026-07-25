#pragma once

#include <cstdint>

namespace eyesee {

/**
 * Placeholder real-time eye-state broadcast. Target update rate 30-60Hz
 * (docs/ROADMAP.md v0.4). No functionality this pass.
 */
class WebSocketServer {
public:
    void begin();
    void update(uint32_t deltaMs);
};

}  // namespace eyesee
