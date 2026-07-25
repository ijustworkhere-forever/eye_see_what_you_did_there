#pragma once

#include <cstdint>

namespace eyesee {

/**
 * Placeholder REST API. Planned versioned routes (docs/ROADMAP.md v0.4),
 * each pushing an EyeCommand into the shared CommandQueue:
 *   GET  /api/v1/status
 *   POST /api/v1/look
 *   POST /api/v1/blink
 *   POST /api/v1/expression
 *   POST /api/v1/config
 *   GET  /api/v1/config
 */
class RestApi {
public:
    void begin();
    void update(uint32_t deltaMs);
};

}  // namespace eyesee
