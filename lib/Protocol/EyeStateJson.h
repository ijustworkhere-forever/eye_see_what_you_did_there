#pragma once

#include <cstdint>
#include <string>

#include "EyeState.h"
#include "EyeTypes.h"

namespace eyesee {

/** Converts an EyeState to its wire string form, e.g. EyeState::Idle -> "Idle". */
const char* eyeStateToString(EyeState state);

/** Converts an Expression to its wire string form, e.g. Expression::Happy -> "Happy". */
const char* expressionToString(Expression expression);

/** Converts a wire string form back to an Expression. Returns false (out left unchanged)
 * if name doesn't match any Expression value. */
bool expressionFromString(const std::string& name, Expression& out);

/** GET /api/v1/status response body: a full diagnostics snapshot. */
std::string buildStatusJson(EyeState state, const EyePose& pose, bool wifiConnected);

/** WebSocket broadcast payload -- deliberately leaner than buildStatusJson (no wifi field),
 * since it's sent at up to 30Hz. */
std::string buildBroadcastJson(EyeState state, const EyePose& pose, uint32_t uptimeMs);

/** Error response body for a failed request: {"error": message}. */
std::string buildErrorJson(const std::string& message);

}  // namespace eyesee
