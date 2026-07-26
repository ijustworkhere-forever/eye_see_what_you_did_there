#pragma once

#include <cstdint>

namespace eyesee {

/**
 * Owns the WiFi STA connection lifecycle: connect once, retry with backoff
 * if dropped. The eye-motion pipeline has zero dependency on WiFi state --
 * a disconnected or never-configured network leaves the eyes fully
 * functional, just unreachable remotely.
 */
class WifiManager {
public:
    /** Starts a STA-mode connection attempt. Call once from setup(). */
    void begin(const char* ssid, const char* password);
    /** Checks connection state; retries every kRetryIntervalMs if not connected. */
    void update(uint32_t deltaMs);
    bool isConnected() const;

private:
    uint32_t msSinceLastAttempt_ = 0;
    static constexpr uint32_t kRetryIntervalMs = 5000;
};

}  // namespace eyesee
