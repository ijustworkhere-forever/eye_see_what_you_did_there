#pragma once

#include <cstdint>

#ifdef ARDUINO

#include <BLEGamepadClient.h>

#include "CommandQueue.h"
#include "GamepadInputMapper.h"

namespace eyesee {

/** Connects to a paired BLE Xbox controller and maps its input to EyeCommands
 * via GamepadInputMapper -- see docs/superpowers/specs/2026-07-26-v0.6-integrations-design.md. */
class GamepadBridge {
public:
    explicit GamepadBridge(CommandQueue& commandQueue);

    /** Starts BLE scanning/auto-connect for a supported controller. Call once from setup(). */
    void begin();
    /** No-op -- BLEGamepadClient's onValueChanged() callback drives this class, not a
     * polled update(); kept for the shared per-frame update() convention. */
    void update(uint32_t deltaMs);

private:
    CommandQueue& commandQueue_;
    XboxController controller_;
    GamepadState previousState_;

    void handleValueChanged(XboxControlsState& state);
};

}  // namespace eyesee

#endif  // ARDUINO
