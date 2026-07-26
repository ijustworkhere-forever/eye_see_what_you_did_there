#pragma once

#include <cstdint>

class AsyncWebServer;

namespace eyesee {

/** Serves data/'s static assets (the Web UI) from LittleFS. */
class WebServer {
public:
    /** Mounts LittleFS and registers the static file handler. Call once from setup(),
     * before server.begin(). */
    void begin(AsyncWebServer& server);
    /** No-op -- ESPAsyncWebServer is request-driven; kept for the shared per-frame
     * update() convention every Networking class follows. */
    void update(uint32_t deltaMs);
};

}  // namespace eyesee
