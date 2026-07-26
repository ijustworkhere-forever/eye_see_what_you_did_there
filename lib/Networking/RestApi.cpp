#include "RestApi.h"

#include <ESPAsyncWebServer.h>

#include "EyeCommandJson.h"
#include "EyeConfigJson.h"
#include "EyeStateJson.h"

namespace eyesee {

namespace {

/** Sleep/wake take no body, so this is the only source of their EyeCommand::durationMs --
 * matching the 500ms SleepBehavior used before v0.3's final review made the command's
 * duration authoritative (see docs/superpowers/sdd notes for that milestone); leaving this
 * at EyeCommand's default 0 would make sleep/wake snap the lids instantly instead of easing. */
constexpr uint32_t kSleepWakeDurationMs = 500;

/** Pushes command; responds 202 on success, 503 if CommandQueue::kCapacity (16) is full. */
void respondQueued(AsyncWebServerRequest* request, CommandQueue& queue, const EyeCommand& command) {
    if (!queue.push(command)) {
        request->send(503, "application/json", buildErrorJson("command queue full").c_str());
        return;
    }
    request->send(202, "application/json", "{\"queued\": true}");
}

}  // namespace

RestApi::RestApi(CommandQueue& commandQueue, const IBehaviorEngine& behaviorEngine,
                 const EyeController& eyeController, const WifiManager& wifiManager,
                 CalibrationManager& calibrationManager, IStorage& storage)
    : commandQueue_(commandQueue),
      behaviorEngine_(behaviorEngine),
      eyeController_(eyeController),
      wifiManager_(wifiManager),
      calibrationManager_(calibrationManager),
      storage_(storage) {
}

void RestApi::begin(AsyncWebServer& server) {
    server.on("/api/v1/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        const std::string body = buildStatusJson(
            behaviorEngine_.state(), eyeController_.currentPose(), wifiManager_.isConnected());
        request->send(200, "application/json", body.c_str());
    });

    server
        .on("/api/v1/look", HTTP_POST,
            [this](AsyncWebServerRequest* request, JsonVariant& json) {
                EyeCommand command;
                const ParseResult result = parseLookCommand(json, command);
                if (!result.ok) {
                    request->send(400, "application/json", buildErrorJson(result.error).c_str());
                    return;
                }
                respondQueued(request, commandQueue_, command);
            })
        .setMaxContentLength(512);

    server
        .on("/api/v1/blink", HTTP_POST,
            [this](AsyncWebServerRequest* request, JsonVariant& json) {
                EyeCommand command;
                parseBlinkCommand(json, command);  // always succeeds
                respondQueued(request, commandQueue_, command);
            })
        .setMaxContentLength(512);

    server
        .on("/api/v1/wink", HTTP_POST,
            [this](AsyncWebServerRequest* request, JsonVariant& json) {
                EyeCommand command;
                const ParseResult result = parseWinkCommand(json, command);
                if (!result.ok) {
                    request->send(400, "application/json", buildErrorJson(result.error).c_str());
                    return;
                }
                respondQueued(request, commandQueue_, command);
            })
        .setMaxContentLength(512);

    server
        .on("/api/v1/expression", HTTP_POST,
            [this](AsyncWebServerRequest* request, JsonVariant& json) {
                EyeCommand command;
                const ParseResult result = parseExpressionCommand(json, command);
                if (!result.ok) {
                    request->send(400, "application/json", buildErrorJson(result.error).c_str());
                    return;
                }
                respondQueued(request, commandQueue_, command);
            })
        .setMaxContentLength(512);

    server
        .on("/api/v1/track", HTTP_POST,
            [this](AsyncWebServerRequest* request, JsonVariant& json) {
                EyeCommand command;
                const ParseResult result = parseTrackCommand(json, command);
                if (!result.ok) {
                    request->send(400, "application/json", buildErrorJson(result.error).c_str());
                    return;
                }
                respondQueued(request, commandQueue_, command);
            })
        .setMaxContentLength(512);

    server.on("/api/v1/sleep", HTTP_POST, [this](AsyncWebServerRequest* request) {
        EyeCommand command;
        command.type = CommandType::Sleep;
        command.durationMs = kSleepWakeDurationMs;
        respondQueued(request, commandQueue_, command);
    });

    server.on("/api/v1/wake", HTTP_POST, [this](AsyncWebServerRequest* request) {
        EyeCommand command;
        command.type = CommandType::Wake;
        command.durationMs = kSleepWakeDurationMs;
        respondQueued(request, commandQueue_, command);
    });

    server.on("/api/v1/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        const std::string body = buildConfigJson(calibrationManager_.eyeConfig());
        request->send(200, "application/json", body.c_str());
    });

    server
        .on("/api/v1/config", HTTP_POST,
            [this](AsyncWebServerRequest* request, JsonVariant& json) {
                const ConfigParseResult result = parseConfigUpdate(json);
                if (!result.ok) {
                    request->send(400, "application/json", buildErrorJson(result.error).c_str());
                    return;
                }
                ServoConfig config = result.servoConfig;
                config.channel = calibrationManager_.servoConfig(result.channel).channel;
                calibrationManager_.setServoConfig(result.channel, config);
                calibrationManager_.saveToStorage(storage_);
                const std::string body = buildConfigJson(calibrationManager_.eyeConfig());
                request->send(200, "application/json", body.c_str());
            })
        .setMaxContentLength(512);
}

void RestApi::update(uint32_t deltaMs) {
    (void)deltaMs;
}

}  // namespace eyesee
