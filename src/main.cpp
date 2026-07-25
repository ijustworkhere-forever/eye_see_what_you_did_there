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
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
    servoOutput.init();
    preferencesStore.begin("eyesee");
    webServer.begin();
    restApi.begin();
    webSocketServer.begin();
    otaManager.begin();
    eyeController.setIdle();
    behaviorEngine.setState(EyeState::Startup);
}

void loop() {
}
