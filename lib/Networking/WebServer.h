#pragma once

#include <cstdint>

namespace eyesee {

/**
 * Placeholder for the Web UI static asset server. No functionality this
 * pass — see docs/ROADMAP.md v0.4.
 */
class WebServer {
public:
    void begin();
    void update(uint32_t deltaMs);
};

}  // namespace eyesee
