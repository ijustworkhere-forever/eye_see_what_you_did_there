#pragma once

#include <cstdint>

// Only the pipeline value types are needed here — deliberately NOT
// EyeController.h, which would drag calibration and servo types into Behavior
// and break "Behavior never knows hardware" (docs/architecture.md invariant 3).
#include "EyeTypes.h"

namespace eyesee {

/** Which action an EyeCommand requests; selects the EyeCommand fields that are valid. */
enum class CommandType {
    Look,
    Blink,
    WinkLeft,
    WinkRight,
    Sleep,
    Wake,
    SetExpression
};

/** Relative urgency of a queued command. Priority-aware arbitration is future work
 * (docs/ROADMAP.md v0.3); CommandQueue is strictly FIFO today. */
enum class CommandPriority {
    Low,
    Normal,
    High
};

/**
 * What every control-plane input (Web UI, REST, WebSocket, Bluetooth,
 * Serial) pushes into CommandQueue. BehaviorEngine is the only consumer.
 */
struct EyeCommand {
    CommandType type = CommandType::Look;
    CommandPriority priority = CommandPriority::Normal;
    uint32_t durationMs = 0;
    GazeTarget gazeTarget{};                      // valid when type == Look
    Expression expression = Expression::Neutral;  // valid when type == SetExpression
};

}  // namespace eyesee
