#pragma once

#include <array>
#include <cstddef>

#include "EyeCommand.h"

namespace eyesee {

/** Library-agnostic gamepad snapshot -- decoupled from any specific BLE gamepad
 * library's concrete type, so this module has zero BLE/Arduino dependency. */
struct GamepadState {
    float leftStickX = 0.0f;
    float leftStickY = 0.0f;
    bool buttonA = false;
    bool buttonB = false;
    bool buttonX = false;
    bool buttonY = false;
    bool leftBumper = false;
    bool rightBumper = false;
};

/** One Look from the stick + one per the six mapped buttons. */
constexpr size_t kMaxCommandsPerFrame = 7;

/** Maps one frame of gamepad state to zero or more EyeCommands, given the previous
 * frame's state (for edge-detecting button presses -- a held button fires its
 * action once, not every frame). Writes into outCommands (fixed capacity, no heap
 * allocation) and returns the count actually written. */
size_t mapGamepadState(const GamepadState& current, const GamepadState& previous,
                       std::array<EyeCommand, kMaxCommandsPerFrame>& outCommands);

}  // namespace eyesee
