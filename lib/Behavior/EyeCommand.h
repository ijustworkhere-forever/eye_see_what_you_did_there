#pragma once

#include <cstdint>

#include "EyeController.h"

namespace eyesee {

enum class CommandType { Look, Blink, WinkLeft, WinkRight, Sleep, Wake, SetExpression };
enum class CommandPriority { Low, Normal, High };

/**
 * What every control-plane input (Web UI, REST, WebSocket, Bluetooth,
 * Serial) pushes into CommandQueue. BehaviorEngine is the only consumer.
 */
struct EyeCommand {
    CommandType type = CommandType::Look;
    CommandPriority priority = CommandPriority::Normal;
    uint32_t durationMs = 0;
    GazeTarget gazeTarget{};   // valid when type == Look
    Expression expression = Expression::Neutral;  // valid when type == SetExpression
};

}  // namespace eyesee
