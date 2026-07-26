#include "GamepadBridge.h"

#ifdef ARDUINO

#include "Logger.h"

namespace eyesee {

namespace {
constexpr const char* kLogTag = "GamepadBridge";
}  // namespace

GamepadBridge::GamepadBridge(CommandQueue& commandQueue) : commandQueue_(commandQueue) {
}

void GamepadBridge::begin() {
    BLEGamepadClient::init();
    controller_.begin();
    controller_.onConnected([](XboxController&) { Logger::info(kLogTag, "gamepad connected"); });
    controller_.onDisconnected([](XboxController&) { Logger::info(kLogTag, "gamepad disconnected"); });
    controller_.onValueChanged([this](XboxControlsState& state) { handleValueChanged(state); });
}

void GamepadBridge::update(uint32_t deltaMs) {
    (void)deltaMs;
}

void GamepadBridge::handleValueChanged(XboxControlsState& state) {
    GamepadState current;
    current.leftStickX = state.leftStickX;
    current.leftStickY = state.leftStickY;
    current.buttonA = state.buttonA;
    current.buttonB = state.buttonB;
    current.buttonX = state.buttonX;
    current.buttonY = state.buttonY;
    current.leftBumper = state.leftBumper;
    current.rightBumper = state.rightBumper;

    std::array<EyeCommand, kMaxCommandsPerFrame> commands{};
    const size_t count = mapGamepadState(current, previousState_, commands);
    for (size_t i = 0; i < count; ++i) {
        if (!commandQueue_.push(commands[i])) {
            Logger::error(kLogTag, "command queue full, dropped gamepad command");
        }
    }

    previousState_ = current;
}

}  // namespace eyesee

#endif  // ARDUINO
