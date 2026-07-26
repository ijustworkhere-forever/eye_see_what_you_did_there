#include "EyeConfigJson.h"

namespace eyesee {

const char* channelToString(EyeChannel channel) {
    switch (channel) {
        case EyeChannel::Lr: return "lr";
        case EyeChannel::Ud: return "ud";
        case EyeChannel::Tl: return "tl";
        case EyeChannel::Bl: return "bl";
        case EyeChannel::Tr: return "tr";
        case EyeChannel::Br: return "br";
    }
    return "lr";
}

bool channelFromString(const std::string& name, EyeChannel& out) {
    if (name == "lr") { out = EyeChannel::Lr; return true; }
    if (name == "ud") { out = EyeChannel::Ud; return true; }
    if (name == "tl") { out = EyeChannel::Tl; return true; }
    if (name == "bl") { out = EyeChannel::Bl; return true; }
    if (name == "tr") { out = EyeChannel::Tr; return true; }
    if (name == "br") { out = EyeChannel::Br; return true; }
    return false;
}

ConfigParseResult parseConfigUpdate(JsonVariantConst body) {
    ConfigParseResult result;

    if (!body["channel"].is<const char*>()) {
        result.error = "missing required field: channel";
        return result;
    }
    if (!channelFromString(body["channel"].as<const char*>(), result.channel)) {
        result.error = "unrecognized channel name";
        return result;
    }

    if (!body["minPulseUs"].is<uint16_t>() || !body["maxPulseUs"].is<uint16_t>() ||
        !body["neutralPulseUs"].is<uint16_t>() || !body["mechanicalOffset"].is<int16_t>() ||
        !body["inverted"].is<bool>() || !body["mirrored"].is<bool>()) {
        result.error =
            "missing required field: minPulseUs, maxPulseUs, neutralPulseUs, "
            "mechanicalOffset, inverted, and mirrored are all required";
        return result;
    }

    result.servoConfig.minPulseUs = body["minPulseUs"].as<uint16_t>();
    result.servoConfig.maxPulseUs = body["maxPulseUs"].as<uint16_t>();
    result.servoConfig.neutralPulseUs = body["neutralPulseUs"].as<uint16_t>();
    result.servoConfig.mechanicalOffset = body["mechanicalOffset"].as<int16_t>();
    result.servoConfig.inverted = body["inverted"].as<bool>();
    result.servoConfig.mirrored = body["mirrored"].as<bool>();
    result.ok = true;
    return result;
}

namespace {
void writeServoConfig(JsonObject& obj, const ServoConfig& config) {
    obj["minPulseUs"] = config.minPulseUs;
    obj["maxPulseUs"] = config.maxPulseUs;
    obj["neutralPulseUs"] = config.neutralPulseUs;
    obj["mechanicalOffset"] = config.mechanicalOffset;
    obj["inverted"] = config.inverted;
    obj["mirrored"] = config.mirrored;
}
}  // namespace

std::string buildConfigJson(const EyeConfig& config) {
    JsonDocument doc;
    JsonObject lr = doc["lr"].to<JsonObject>();
    writeServoConfig(lr, config.lr);
    JsonObject ud = doc["ud"].to<JsonObject>();
    writeServoConfig(ud, config.ud);
    JsonObject tl = doc["tl"].to<JsonObject>();
    writeServoConfig(tl, config.tl);
    JsonObject bl = doc["bl"].to<JsonObject>();
    writeServoConfig(bl, config.bl);
    JsonObject tr = doc["tr"].to<JsonObject>();
    writeServoConfig(tr, config.tr);
    JsonObject br = doc["br"].to<JsonObject>();
    writeServoConfig(br, config.br);
    doc["lookRangeDegrees"] = config.lookRangeDegrees;

    std::string output;
    serializeJson(doc, output);
    return output;
}

}  // namespace eyesee
