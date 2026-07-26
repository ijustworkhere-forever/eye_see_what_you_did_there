#include "GamepadInputMapper.h"

namespace eyesee {

namespace {
constexpr float kStickDeadzone = 0.1f;

// Matches lib/Protocol/EyeCommandJson.cpp (blink/wink default to 150, expression to 200) and
// lib/Networking/RestApi.cpp / lib/Protocol/MqttCommandJson.cpp's kSleepWakeDurationMs (500) --
// duplicated locally rather than shared via include since this file has zero
// Arduino/BLE dependency and shouldn't gain a cross-module include just for these numbers.
constexpr uint32_t kBlinkWinkDurationMs = 150;
constexpr uint32_t kExpressionDurationMs = 200;
constexpr uint32_t kSleepWakeDurationMs = 500;

bool pressEdge(bool current, bool previous) {
    return current && !previous;
}
}  // namespace

size_t mapGamepadState(const GamepadState& current, const GamepadState& previous,
                       std::array<EyeCommand, kMaxCommandsPerFrame>& outCommands) {
    size_t count = 0;

    if (current.leftStickX > kStickDeadzone || current.leftStickX < -kStickDeadzone ||
        current.leftStickY > kStickDeadzone || current.leftStickY < -kStickDeadzone) {
        EyeCommand command;
        command.type = CommandType::Look;
        command.gazeTarget.x = current.leftStickX;
        command.gazeTarget.y = current.leftStickY;
        outCommands[count++] = command;
    }

    if (pressEdge(current.buttonA, previous.buttonA)) {
        EyeCommand command;
        command.type = CommandType::Blink;
        command.durationMs = kBlinkWinkDurationMs;
        outCommands[count++] = command;
    }

    if (pressEdge(current.leftBumper, previous.leftBumper)) {
        EyeCommand command;
        command.type = CommandType::WinkLeft;
        command.durationMs = kBlinkWinkDurationMs;
        outCommands[count++] = command;
    }

    if (pressEdge(current.rightBumper, previous.rightBumper)) {
        EyeCommand command;
        command.type = CommandType::WinkRight;
        command.durationMs = kBlinkWinkDurationMs;
        outCommands[count++] = command;
    }

    if (pressEdge(current.buttonY, previous.buttonY)) {
        EyeCommand command;
        command.type = CommandType::SetExpression;
        command.expression = Expression::Happy;
        command.durationMs = kExpressionDurationMs;
        outCommands[count++] = command;
    }

    if (pressEdge(current.buttonB, previous.buttonB)) {
        EyeCommand command;
        command.type = CommandType::Sleep;
        command.durationMs = kSleepWakeDurationMs;
        outCommands[count++] = command;
    }

    if (pressEdge(current.buttonX, previous.buttonX)) {
        EyeCommand command;
        command.type = CommandType::Wake;
        command.durationMs = kSleepWakeDurationMs;
        outCommands[count++] = command;
    }

    return count;
}

}  // namespace eyesee
