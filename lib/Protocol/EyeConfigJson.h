#pragma once

#include <string>

#include <ArduinoJson.h>

#include "CalibrationManager.h"
#include "Configuration.h"

namespace eyesee {

/** Converts an EyeChannel to its wire string form, e.g. EyeChannel::Lr -> "lr". */
const char* channelToString(EyeChannel channel);

/** Converts a wire string form back to an EyeChannel. Returns false (out unchanged)
 * if name doesn't match any EyeChannel value. */
bool channelFromString(const std::string& name, EyeChannel& out);

/** Outcome of parsing a POST /api/v1/config body. `error` set only when `ok` is false;
 * `channel`/`servoConfig` valid only when `ok` is true. */
struct ConfigParseResult {
    bool ok = false;
    std::string error;
    EyeChannel channel = EyeChannel::Lr;
    ServoConfig servoConfig;
};

/** {"channel": "lr", "minPulseUs", "maxPulseUs", "neutralPulseUs", "mechanicalOffset",
 *  "inverted", "mirrored"} -- every field required, no partial patch. */
ConfigParseResult parseConfigUpdate(JsonVariantConst body);

/** GET /api/v1/config response body: full EyeConfig snapshot, keyed by channelToString(). */
std::string buildConfigJson(const EyeConfig& config);

}  // namespace eyesee
