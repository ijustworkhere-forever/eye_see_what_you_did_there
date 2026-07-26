#pragma once

#include <cstdint>

namespace eyesee {

/** Wraps ArduinoOTA: lets the device be reflashed over WiFi instead of USB. */
class OtaManager {
public:
    /** Starts the ArduinoOTA listener. Call once from setup(), any time after
     * WifiManager::begin() -- ArduinoOTA doesn't require an active connection
     * yet at call time, only once a client actually attempts an update. */
    void begin();
    /** Services pending OTA requests. Cheap no-op when no update is in
     * progress or WiFi is disconnected -- no rate-limiting needed. */
    void update(uint32_t deltaMs);
};

}  // namespace eyesee
