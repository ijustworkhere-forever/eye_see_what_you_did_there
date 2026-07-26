#include <unity.h>

#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>

#include "EyeStateJson.h"

using eyesee::buildBroadcastJson;
using eyesee::buildErrorJson;
using eyesee::buildStatusJson;
using eyesee::EyePose;
using eyesee::EyeState;
using eyesee::Expression;
using eyesee::eyeStateToString;
using eyesee::expressionFromString;
using eyesee::expressionToString;

void test_eye_state_to_string_covers_every_value() {
    TEST_ASSERT_EQUAL_STRING("Startup", eyeStateToString(EyeState::Startup));
    TEST_ASSERT_EQUAL_STRING("Calibration", eyeStateToString(EyeState::Calibration));
    TEST_ASSERT_EQUAL_STRING("Manual", eyeStateToString(EyeState::Manual));
    TEST_ASSERT_EQUAL_STRING("Idle", eyeStateToString(EyeState::Idle));
    TEST_ASSERT_EQUAL_STRING("Tracking", eyeStateToString(EyeState::Tracking));
    TEST_ASSERT_EQUAL_STRING("Sleeping", eyeStateToString(EyeState::Sleeping));
    TEST_ASSERT_EQUAL_STRING("Disabled", eyeStateToString(EyeState::Disabled));
    TEST_ASSERT_EQUAL_STRING("Error", eyeStateToString(EyeState::Error));
}

void test_expression_string_round_trips_every_value() {
    const Expression values[] = {Expression::Neutral, Expression::Happy, Expression::Curious,
                                  Expression::Sleepy,  Expression::Angry, Expression::Surprised};
    for (Expression value : values) {
        const char* name = expressionToString(value);
        Expression parsed = Expression::Neutral;
        TEST_ASSERT_TRUE(expressionFromString(name, parsed));
        TEST_ASSERT_EQUAL_INT(static_cast<int>(value), static_cast<int>(parsed));
    }
}

void test_expression_from_string_rejects_unknown_name() {
    Expression parsed = Expression::Neutral;
    TEST_ASSERT_FALSE(expressionFromString("Grumpy", parsed));
}

void test_build_status_json_reports_state_pose_and_wifi() {
    EyePose pose;
    pose.lookX = 0.25f;
    pose.lookY = -0.5f;
    pose.upperLeftLid = 0.9f;
    pose.lowerLeftLid = 0.8f;
    pose.upperRightLid = 0.7f;
    pose.lowerRightLid = 0.6f;

    const std::string json = buildStatusJson(EyeState::Tracking, pose, true);

    JsonDocument doc;
    TEST_ASSERT_FALSE(deserializeJson(doc, json));
    TEST_ASSERT_EQUAL_STRING("Tracking", doc["state"].as<const char*>());
    TEST_ASSERT_TRUE(doc["wifiConnected"].as<bool>());
    TEST_ASSERT_EQUAL_FLOAT(0.25f, doc["pose"]["lookX"].as<float>());
    TEST_ASSERT_EQUAL_FLOAT(-0.5f, doc["pose"]["lookY"].as<float>());
    TEST_ASSERT_EQUAL_FLOAT(0.9f, doc["pose"]["upperLeftLid"].as<float>());
    TEST_ASSERT_EQUAL_FLOAT(0.8f, doc["pose"]["lowerLeftLid"].as<float>());
    TEST_ASSERT_EQUAL_FLOAT(0.7f, doc["pose"]["upperRightLid"].as<float>());
    TEST_ASSERT_EQUAL_FLOAT(0.6f, doc["pose"]["lowerRightLid"].as<float>());
}

void test_build_broadcast_json_omits_wifi_field() {
    EyePose pose;
    const std::string json = buildBroadcastJson(EyeState::Idle, pose, 12345);

    JsonDocument doc;
    TEST_ASSERT_FALSE(deserializeJson(doc, json));
    TEST_ASSERT_EQUAL_STRING("Idle", doc["state"].as<const char*>());
    TEST_ASSERT_EQUAL_UINT32(12345, doc["uptimeMs"].as<uint32_t>());
    TEST_ASSERT_TRUE(doc["wifiConnected"].isNull());
}

void test_build_error_json_wraps_message() {
    const std::string json = buildErrorJson("bad request");

    JsonDocument doc;
    TEST_ASSERT_FALSE(deserializeJson(doc, json));
    TEST_ASSERT_EQUAL_STRING("bad request", doc["error"].as<const char*>());
}

// No main() here — test/test_native/test_main.cpp is the sole file with main().
