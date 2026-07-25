#pragma once

#include <cstdint>

namespace eyesee {

/** Placeholder OTA update manager. No functionality this pass — see docs/ROADMAP.md v0.5. */
class OtaManager {
public:
    void begin();
    void update(uint32_t deltaMs);
};

}  // namespace eyesee
