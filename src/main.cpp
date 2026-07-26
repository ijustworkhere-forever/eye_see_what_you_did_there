#include <Arduino.h>

#include "BehaviorEngine.h"
#include "CalibrationManager.h"
#include "CommandQueue.h"
#include "EyeController.h"
#include "IdleBehaviorStub.h"
#include "Logger.h"
#include "OtaManager.h"
#include "Pca9685ServoOutput.h"
#include "PreferencesStore.h"
#include "RealAnimationEngine.h"
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
RealAnimationEngine animationEngine(eyeController, calibration);
CommandQueue commandQueue;
IdleBehaviorStub idleBehavior;
BehaviorEngine behaviorEngine(animationEngine, commandQueue, idleBehavior);

PreferencesStore preferencesStore;
WebServer webServer;
RestApi restApi;
WebSocketServer webSocketServer;
OtaManager otaManager;

uint32_t lastFrameMillis = 0;
constexpr uint32_t kFramePeriodMs = 10;  // 100Hz target

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
    const uint32_t nowMillis = millis();
    const uint32_t elapsed = nowMillis - lastFrameMillis;  // wraps correctly via unsigned arithmetic
    if (elapsed < kFramePeriodMs) {
        return;
    }
    lastFrameMillis = nowMillis;

    behaviorEngine.update(elapsed);
    animationEngine.update(elapsed);
    eyeController.update(elapsed);
    servoOutput.update(elapsed);

    webServer.update(elapsed);
    restApi.update(elapsed);
    webSocketServer.update(elapsed);
    otaManager.update(elapsed);
}
