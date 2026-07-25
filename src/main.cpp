#include <Arduino.h>

#include "Logger.h"
#include "Pca9685ServoOutput.h"

using namespace eyesee;

namespace {
constexpr const char* kLogTag = "main";
Pca9685ServoOutput servoOutput;
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
    servoOutput.init();
}

void loop() {
}
