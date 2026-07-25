#ifdef ARDUINO
#include <Arduino.h>

#include "CalibrationManager.h"
#include "EyeController.h"
#include "Logger.h"
#include "Pca9685ServoOutput.h"

using namespace eyesee;

namespace {
constexpr const char* kLogTag = "main";
Pca9685ServoOutput servoOutput;
CalibrationManager calibration;
EyeController eyeController(servoOutput, calibration);
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
    servoOutput.init();
    eyeController.setIdle();
}

void loop() {
}
#endif  // ARDUINO
