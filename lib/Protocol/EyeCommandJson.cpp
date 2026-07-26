#include "EyeCommandJson.h"

#include "EyeStateJson.h"

namespace eyesee {

namespace {
float clampf(float value, float lo, float hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}
}  // namespace

ParseResult parseLookCommand(JsonVariantConst body, EyeCommand& out) {
    if (!body["x"].is<float>() || !body["y"].is<float>()) {
        return ParseResult{false, "missing required field: x and y must be numbers"};
    }

    GazeTarget target;
    target.x = clampf(body["x"].as<float>(), -1.0f, 1.0f);
    target.y = clampf(body["y"].as<float>(), -1.0f, 1.0f);
    target.speed = body["speed"] | target.speed;
    target.blinkOnArrival = body["blinkOnArrival"] | target.blinkOnArrival;
    target.hold = body["hold"] | target.hold;

    out = EyeCommand{};
    out.type = CommandType::Look;
    out.gazeTarget = target;
    return ParseResult{true, ""};
}

ParseResult parseBlinkCommand(JsonVariantConst body, EyeCommand& out) {
    out = EyeCommand{};
    out.type = CommandType::Blink;
    out.durationMs = body["durationMs"] | static_cast<uint32_t>(150);
    return ParseResult{true, ""};
}

ParseResult parseWinkCommand(JsonVariantConst body, EyeCommand& out) {
    if (!body["side"].is<const char*>()) {
        return ParseResult{false, "missing required field: side"};
    }
    const std::string side = body["side"].as<const char*>();

    out = EyeCommand{};
    if (side == "left") {
        out.type = CommandType::WinkLeft;
    } else if (side == "right") {
        out.type = CommandType::WinkRight;
    } else {
        return ParseResult{false, "side must be \"left\" or \"right\""};
    }
    out.durationMs = body["durationMs"] | static_cast<uint32_t>(150);
    return ParseResult{true, ""};
}

ParseResult parseExpressionCommand(JsonVariantConst body, EyeCommand& out) {
    if (!body["expression"].is<const char*>()) {
        return ParseResult{false, "missing required field: expression"};
    }

    Expression expression = Expression::Neutral;
    if (!expressionFromString(body["expression"].as<const char*>(), expression)) {
        return ParseResult{false, "unrecognized expression name"};
    }

    out = EyeCommand{};
    out.type = CommandType::SetExpression;
    out.expression = expression;
    out.durationMs = body["durationMs"] | static_cast<uint32_t>(200);
    return ParseResult{true, ""};
}

ParseResult parseTrackCommand(JsonVariantConst body, EyeCommand& out) {
    if (!body["x"].is<float>() || !body["y"].is<float>()) {
        return ParseResult{false, "missing required field: x and y must be numbers"};
    }

    GazeTarget target;
    target.x = clampf(body["x"].as<float>(), -1.0f, 1.0f);
    target.y = clampf(body["y"].as<float>(), -1.0f, 1.0f);
    target.hold = body["hold"] | true;

    out = EyeCommand{};
    out.type = CommandType::Track;
    out.gazeTarget = target;
    return ParseResult{true, ""};
}

}  // namespace eyesee
