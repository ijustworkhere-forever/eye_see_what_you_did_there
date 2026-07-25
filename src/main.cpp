#include <Arduino.h>

#include "BehaviorEngine.h"
#include "CalibrationManager.h"
#include "CommandQueue.h"
#include "EyeController.h"
#include "IdleBehaviorStub.h"
#include "Logger.h"
#include "Pca9685ServoOutput.h"
#include "PassthroughAnimationEngine.h"
#include "PreferencesStore.h"

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
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
    servoOutput.init();
    preferencesStore.begin("eyesee");
    eyeController.setIdle();
    behaviorEngine.setState(EyeState::Startup);
}

void loop() {
}
