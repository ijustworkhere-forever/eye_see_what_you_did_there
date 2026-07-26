#include "WebServer.h"

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

namespace eyesee {

void WebServer::begin(AsyncWebServer& server) {
    LittleFS.begin();
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
}

void WebServer::update(uint32_t deltaMs) {
    (void)deltaMs;
}

}  // namespace eyesee
