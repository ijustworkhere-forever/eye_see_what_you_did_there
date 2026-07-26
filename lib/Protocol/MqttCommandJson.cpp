#include "MqttCommandJson.h"

#include "EyeCommandJson.h"

namespace eyesee {

namespace {
constexpr uint32_t kSleepWakeDurationMs =
    500;  // matches RestApi.cpp's kSleepWakeDurationMs exactly
}  // namespace

MqttParseResult parseMqttCommand(JsonVariantConst body) {
    MqttParseResult result;

    if (!body["type"].is<const char*>()) {
        result.error = "missing required field: type";
        return result;
    }
    const std::string type = body["type"].as<const char*>();

    ParseResult underlying;
    if (type == "look") {
        underlying = parseLookCommand(body, result.command);
    } else if (type == "blink") {
        underlying = parseBlinkCommand(body, result.command);
    } else if (type == "wink") {
        underlying = parseWinkCommand(body, result.command);
    } else if (type == "expression") {
        underlying = parseExpressionCommand(body, result.command);
    } else if (type == "track") {
        underlying = parseTrackCommand(body, result.command);
    } else if (type == "sleep") {
        result.command = EyeCommand{};
        result.command.type = CommandType::Sleep;
        result.command.durationMs = kSleepWakeDurationMs;
        result.ok = true;
        return result;
    } else if (type == "wake") {
        result.command = EyeCommand{};
        result.command.type = CommandType::Wake;
        result.command.durationMs = kSleepWakeDurationMs;
        result.ok = true;
        return result;
    } else {
        result.error = "unrecognized type";
        return result;
    }

    result.ok = underlying.ok;
    result.error = underlying.error;
    return result;
}

}  // namespace eyesee
