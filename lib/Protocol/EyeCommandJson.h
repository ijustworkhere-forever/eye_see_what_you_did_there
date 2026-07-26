#pragma once

#include <string>

#include <ArduinoJson.h>

#include "EyeCommand.h"

namespace eyesee {

/** Outcome of parsing a request body into an EyeCommand. `error` is set only when `ok` is false. */
struct ParseResult {
    bool ok = false;
    std::string error;
};

/** {"x": float, "y": float, "speed"?, "blinkOnArrival"?, "hold"?} -> CommandType::Look. */
ParseResult parseLookCommand(JsonVariantConst body, EyeCommand& out);

/** {"durationMs"?: uint32_t} (default 150) -> CommandType::Blink. Always succeeds. */
ParseResult parseBlinkCommand(JsonVariantConst body, EyeCommand& out);

/** {"side": "left"|"right", "durationMs"?: uint32_t} (default 150) -> CommandType::WinkLeft/WinkRight. */
ParseResult parseWinkCommand(JsonVariantConst body, EyeCommand& out);

/** {"expression": "Happy", "durationMs"?: uint32_t} (default 200) -> CommandType::SetExpression. */
ParseResult parseExpressionCommand(JsonVariantConst body, EyeCommand& out);

/** {"x": float, "y": float, "hold"?: bool} (hold default true) -> CommandType::Track. */
ParseResult parseTrackCommand(JsonVariantConst body, EyeCommand& out);

}  // namespace eyesee
