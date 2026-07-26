#include "GamepadInputMapper.h"

namespace eyesee {

namespace {
constexpr float kStickDeadzone = 0.1f;

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
        outCommands[count++] = command;
    }

    if (pressEdge(current.leftBumper, previous.leftBumper)) {
        EyeCommand command;
        command.type = CommandType::WinkLeft;
        outCommands[count++] = command;
    }

    if (pressEdge(current.rightBumper, previous.rightBumper)) {
        EyeCommand command;
        command.type = CommandType::WinkRight;
        outCommands[count++] = command;
    }

    if (pressEdge(current.buttonY, previous.buttonY)) {
        EyeCommand command;
        command.type = CommandType::SetExpression;
        command.expression = Expression::Happy;
        outCommands[count++] = command;
    }

    if (pressEdge(current.buttonB, previous.buttonB)) {
        EyeCommand command;
        command.type = CommandType::Sleep;
        outCommands[count++] = command;
    }

    if (pressEdge(current.buttonX, previous.buttonX)) {
        EyeCommand command;
        command.type = CommandType::Wake;
        outCommands[count++] = command;
    }

    return count;
}

}  // namespace eyesee
