#include <Arduino.h>

#include "Logger.h"

using namespace eyesee;

namespace {
constexpr const char* kLogTag = "main";
}

void setup() {
    Logger::init();
    Logger::info(kLogTag, "EyeSee firmware booting");
}

void loop() {
}
