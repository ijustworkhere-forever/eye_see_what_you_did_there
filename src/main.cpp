#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "ArduinoRandomSource.h"
#include "BehaviorEngine.h"
#include "CalibrationManager.h"
#include "CommandQueue.h"
#include "CuriousBehavior.h"
#include "EyeController.h"
#include "GamepadBridge.h"
#include "IdleBehavior.h"
#include "Logger.h"
#include "MqttBridge.h"
#include "MqttCredentials.h"
#include "OtaManager.h"
#include "Pca9685ServoOutput.h"
#include "PreferencesStore.h"
#include "RandomBehavior.h"
#include "RealAnimationEngine.h"
#include "RestApi.h"
#include "SleepBehavior.h"
#include "TrackingBehavior.h"
#include "WebServer.h"
#include "WebSocketServer.h"
#include "WifiCredentials.h"
#include "WifiManager.h"

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

ArduinoRandomSource randomSource;
IdleBehavior idleBehavior(randomSource);
SleepBehavior sleepBehavior;
TrackingBehavior trackingBehavior;
CuriousBehavior curiousBehavior(
    randomSource);  // built + tested, not registered — see
                    // docs/superpowers/specs/2026-07-25-v0.3-behavior-design.md
RandomBehavior randomBehavior(randomSource);  // built + tested, not registered — same reason

BehaviorEngine behaviorEngine(animationEngine, commandQueue, idleBehavior);

PreferencesStore preferencesStore;
NetworkConfig networkConfig{kWifiSsid, kWifiPassword, 80};
AsyncWebServer server(networkConfig.webServerPort);
WifiManager wifiManager;
WebServer webServer;
RestApi restApi(commandQueue, behaviorEngine, eyeController, wifiManager, calibration,
                preferencesStore);
WebSocketServer webSocketServer(behaviorEngine, eyeController);
OtaManager otaManager;
MqttBridge mqttBridge(commandQueue, behaviorEngine, eyeController, wifiManager);
GamepadBridge gamepadBridge(commandQueue);

uint32_t lastFrameMillis = 0;
constexpr uint32_t kFramePeriodMs = 10;  // 100Hz target

}  // namespace

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");

    servoOutput.init();
    Logger::info(kLogTag, "servoOutput.init() done");

    preferencesStore.begin("eyesee");
    calibration.loadFromStorage(preferencesStore);
    Logger::info(kLogTag, "storage/calibration loaded");

    // TEMPORARY DIAGNOSTIC: WiFi radio power-on alone (WiFi.mode(), before any
    // connect attempt) already trips the brownout -- confirmed by a prior test.
    // This is now a hardware power-delivery problem, not something WiFi/BLE
    // tuning can fix. Skip the entire networking + BLE stack here so the rest
    // of the system (servos/PCA9685/mechanism) can be verified independently
    // while that gets sorted. Revert this whole block once power is fixed.
    Logger::info(kLogTag, "networking/BLE stack SKIPPED for diagnostic test");

    behaviorEngine.registerBehavior(EyeState::Idle, idleBehavior);
    behaviorEngine.registerBehavior(EyeState::Sleeping, sleepBehavior);
    behaviorEngine.registerBehavior(EyeState::Tracking, trackingBehavior);

    behaviorEngine.setState(EyeState::Startup);
    eyeController.setIdle();
    Logger::info(kLogTag, "eyeController.setIdle() done (all 6 servos commanded)");
    behaviorEngine.setState(EyeState::Idle);

    lastFrameMillis = millis();
    Logger::info(kLogTag, "EyeSee firmware ready");
}

void loop() {
    const uint32_t nowMillis = millis();
    const uint32_t elapsed =
        nowMillis - lastFrameMillis;  // wraps correctly via unsigned arithmetic
    if (elapsed < kFramePeriodMs) {
        delay(1);  // yield to the idle/WiFi tasks instead of busy-spinning; 1ms << kFramePeriodMs
        return;
    }
    lastFrameMillis = nowMillis;

    behaviorEngine.update(elapsed);
    animationEngine.update(elapsed);
    eyeController.update(elapsed);
    servoOutput.update(elapsed);

    // TEMPORARY DIAGNOSTIC: matching skip for setup()'s networking/BLE skip --
    // none of these had begin() called, so don't call update() on them either.
    // wifiManager.update(elapsed);
    // webServer.update(elapsed);
    // restApi.update(elapsed);
    // webSocketServer.update(elapsed);
    // otaManager.update(elapsed);
    // mqttBridge.update(elapsed);
    // gamepadBridge.update(elapsed);
}
