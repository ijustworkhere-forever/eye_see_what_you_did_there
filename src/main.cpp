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
    preferencesStore.begin("eyesee");
    calibration.loadFromStorage(preferencesStore);
    wifiManager.begin(networkConfig.ssid, networkConfig.password);
    webServer.begin(server);
    restApi.begin(server);
    webSocketServer.begin(server);
    server.begin();
    otaManager.begin();
    mqttBridge.begin(kMqttBrokerHost, kMqttBrokerPort, kMqttTopicPrefix);
    gamepadBridge.begin();

    behaviorEngine.registerBehavior(EyeState::Idle, idleBehavior);
    behaviorEngine.registerBehavior(EyeState::Sleeping, sleepBehavior);
    behaviorEngine.registerBehavior(EyeState::Tracking, trackingBehavior);

    behaviorEngine.setState(EyeState::Startup);
    eyeController.setIdle();
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

    wifiManager.update(elapsed);
    webServer.update(elapsed);
    restApi.update(elapsed);
    webSocketServer.update(elapsed);
    otaManager.update(elapsed);
    mqttBridge.update(elapsed);
    gamepadBridge.update(elapsed);
}
