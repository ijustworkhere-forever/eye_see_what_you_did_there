#include "WebServer.h"

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include "Logger.h"

namespace eyesee {

namespace {
constexpr const char* kLogTag = "WebServer";
}  // namespace

void WebServer::begin(AsyncWebServer& server) {
    if (!LittleFS.begin()) {
        Logger::error(kLogTag,
                       "LittleFS mount failed -- flash the filesystem image with 'pio run -t uploadfs'");
    }
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
}

void WebServer::update(uint32_t deltaMs) {
    (void)deltaMs;
}

}  // namespace eyesee
