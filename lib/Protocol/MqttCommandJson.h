#pragma once

#include <string>

#include <ArduinoJson.h>

#include "EyeCommand.h"

namespace eyesee {

/** Outcome of parsing one MQTT command-topic message. `error` set only when `ok` is false. */
struct MqttParseResult {
    bool ok = false;
    std::string error;
    EyeCommand command;
};

/** Reads body["type"] ("look"|"blink"|"wink"|"expression"|"sleep"|"wake"|"track") and
 * dispatches to the matching lib/Protocol/EyeCommandJson parse function. */
MqttParseResult parseMqttCommand(JsonVariantConst body);

}  // namespace eyesee
