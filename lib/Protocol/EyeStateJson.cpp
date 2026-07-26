#include "EyeStateJson.h"

#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>

namespace eyesee {

const char* eyeStateToString(EyeState state) {
    switch (state) {
        case EyeState::Startup: return "Startup";
        case EyeState::Calibration: return "Calibration";
        case EyeState::Manual: return "Manual";
        case EyeState::Idle: return "Idle";
        case EyeState::Tracking: return "Tracking";
        case EyeState::Sleeping: return "Sleeping";
        case EyeState::Disabled: return "Disabled";
        case EyeState::Error: return "Error";
    }
    return "Error";
}

const char* expressionToString(Expression expression) {
    switch (expression) {
        case Expression::Neutral: return "Neutral";
        case Expression::Happy: return "Happy";
        case Expression::Curious: return "Curious";
        case Expression::Sleepy: return "Sleepy";
        case Expression::Angry: return "Angry";
        case Expression::Surprised: return "Surprised";
    }
    return "Neutral";
}

bool expressionFromString(const std::string& name, Expression& out) {
    if (name == "Neutral") { out = Expression::Neutral; return true; }
    if (name == "Happy") { out = Expression::Happy; return true; }
    if (name == "Curious") { out = Expression::Curious; return true; }
    if (name == "Sleepy") { out = Expression::Sleepy; return true; }
    if (name == "Angry") { out = Expression::Angry; return true; }
    if (name == "Surprised") { out = Expression::Surprised; return true; }
    return false;
}

namespace {
void writePose(JsonObject& poseObj, const EyePose& pose) {
    poseObj["lookX"] = pose.lookX;
    poseObj["lookY"] = pose.lookY;
    poseObj["upperLeftLid"] = pose.upperLeftLid;
    poseObj["lowerLeftLid"] = pose.lowerLeftLid;
    poseObj["upperRightLid"] = pose.upperRightLid;
    poseObj["lowerRightLid"] = pose.lowerRightLid;
}
}  // namespace

std::string buildStatusJson(EyeState state, const EyePose& pose, bool wifiConnected) {
    JsonDocument doc;
    doc["state"] = eyeStateToString(state);
    doc["wifiConnected"] = wifiConnected;
    JsonObject poseObj = doc["pose"].to<JsonObject>();
    writePose(poseObj, pose);
    std::string output;
    serializeJson(doc, output);
    return output;
}

std::string buildBroadcastJson(EyeState state, const EyePose& pose, uint32_t uptimeMs) {
    JsonDocument doc;
    doc["state"] = eyeStateToString(state);
    doc["uptimeMs"] = uptimeMs;
    JsonObject poseObj = doc["pose"].to<JsonObject>();
    writePose(poseObj, pose);
    std::string output;
    serializeJson(doc, output);
    return output;
}

std::string buildErrorJson(const std::string& message) {
    JsonDocument doc;
    doc["error"] = message;
    std::string output;
    serializeJson(doc, output);
    return output;
}

}  // namespace eyesee
