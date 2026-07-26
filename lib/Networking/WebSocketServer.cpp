#include "WebSocketServer.h"

#include "EyeStateJson.h"

namespace eyesee {

WebSocketServer::WebSocketServer(const IBehaviorEngine& behaviorEngine, const EyeController& eyeController)
    : behaviorEngine_(behaviorEngine), eyeController_(eyeController) {
}

void WebSocketServer::begin(AsyncWebServer& server) {
    server.addHandler(&ws_);
}

void WebSocketServer::update(uint32_t deltaMs) {
    uptimeMs_ += deltaMs;
    ws_.cleanupClients();

    msSinceLastBroadcast_ += deltaMs;
    if (msSinceLastBroadcast_ < kBroadcastPeriodMs) {
        return;
    }
    msSinceLastBroadcast_ = 0;

    const std::string payload =
        buildBroadcastJson(behaviorEngine_.state(), eyeController_.currentPose(), uptimeMs_);
    ws_.textAll(payload.c_str());
}

}  // namespace eyesee
