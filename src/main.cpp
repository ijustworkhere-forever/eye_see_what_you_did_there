#include <Arduino.h>

#include "BehaviorEngine.h"
#include "CalibrationManager.h"
#include "CommandQueue.h"
#include "EyeController.h"
#include "IdleBehaviorStub.h"
#include "Logger.h"
#include "OtaManager.h"
#include "Pca9685ServoOutput.h"
#include "PassthroughAnimationEngine.h"
#include "PreferencesStore.h"
#include "RestApi.h"
#include "WebServer.h"
#include "WebSocketServer.h"

using namespace eyesee;

// This file is the firmware's composition root — the one place file-scope
// objects are acceptable (see docs/architecture.md). Everything below is
// wiring: no business logic lives here.
namespace {

constexpr const char* kLogTag = "main";

Pca9685ServoOutput servoOutput;
CalibrationManager calibration;
EyeController eyeController(servoOutput, calibration);
PassthroughAnimationEngine animationEngine(eyeController);
CommandQueue commandQueue;
IdleBehaviorStub idleBehavior;
BehaviorEngine behaviorEngine(animationEngine, commandQueue, idleBehavior);

PreferencesStore preferencesStore;
WebServer webServer;
RestApi restApi;
WebSocketServer webSocketServer;
OtaManager otaManager;

uint32_t lastFrameMillis = 0;

uint32_t tickDeltaMs() {
    const uint32_t nowMillis = millis();
    const uint32_t deltaMs =
        nowMillis - lastFrameMillis;  // wraps correctly via unsigned arithmetic
    lastFrameMillis = nowMillis;
    return deltaMs;
}

}  // namespace

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");

    servoOutput.init();
    preferencesStore.begin("eyesee");
    webServer.begin();
    restApi.begin();
    webSocketServer.begin();
    otaManager.begin();

    behaviorEngine.setState(EyeState::Startup);
    eyeController.setIdle();
    behaviorEngine.setState(EyeState::Idle);

    lastFrameMillis = millis();
    Logger::info(kLogTag, "EyeSee firmware ready");
}

void loop() {
    // TODO: enforce ~100Hz frame pacing (docs/ROADMAP.md v0.2) — currently runs
    // as fast as the scheduler allows
    const uint32_t deltaMs = tickDeltaMs();

    behaviorEngine.update(deltaMs);
    animationEngine.update(deltaMs);
    eyeController.update(deltaMs);
    servoOutput.update(deltaMs);

    webServer.update(deltaMs);
    restApi.update(deltaMs);
    webSocketServer.update(deltaMs);
    otaManager.update(deltaMs);
}
