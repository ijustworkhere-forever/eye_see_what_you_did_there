#include <unity.h>

#include <ArduinoJson.h>

#include "MqttCommandJson.h"

using eyesee::CommandType;
using eyesee::MqttParseResult;
using eyesee::parseMqttCommand;

void test_parse_mqtt_command_dispatches_look() {
    JsonDocument doc;
    doc["type"] = "look";
    doc["x"] = 0.2f;
    doc["y"] = 0.4f;

    const MqttParseResult result = parseMqttCommand(doc.as<JsonVariantConst>());

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Look), static_cast<int>(result.command.type));
    TEST_ASSERT_EQUAL_FLOAT(0.2f, result.command.gazeTarget.x);
}

void test_parse_mqtt_command_dispatches_wink() {
    JsonDocument doc;
    doc["type"] = "wink";
    doc["side"] = "left";

    const MqttParseResult result = parseMqttCommand(doc.as<JsonVariantConst>());

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::WinkLeft), static_cast<int>(result.command.type));
}

void test_parse_mqtt_command_dispatches_track() {
    JsonDocument doc;
    doc["type"] = "track";
    doc["x"] = 0.1f;
    doc["y"] = -0.1f;

    const MqttParseResult result = parseMqttCommand(doc.as<JsonVariantConst>());

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Track), static_cast<int>(result.command.type));
}

void test_parse_mqtt_command_sleep_needs_no_body_and_gets_easing_duration() {
    JsonDocument doc;
    doc["type"] = "sleep";

    const MqttParseResult result = parseMqttCommand(doc.as<JsonVariantConst>());

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Sleep), static_cast<int>(result.command.type));
    TEST_ASSERT_EQUAL_UINT32(500, result.command.durationMs);
}

void test_parse_mqtt_command_wake_needs_no_body_and_gets_easing_duration() {
    JsonDocument doc;
    doc["type"] = "wake";

    const MqttParseResult result = parseMqttCommand(doc.as<JsonVariantConst>());

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Wake), static_cast<int>(result.command.type));
    TEST_ASSERT_EQUAL_UINT32(500, result.command.durationMs);
}

void test_parse_mqtt_command_rejects_missing_type() {
    JsonDocument doc;
    doc["x"] = 0.1f;

    const MqttParseResult result = parseMqttCommand(doc.as<JsonVariantConst>());

    TEST_ASSERT_FALSE(result.ok);
}

void test_parse_mqtt_command_rejects_unrecognized_type() {
    JsonDocument doc;
    doc["type"] = "dance";

    const MqttParseResult result = parseMqttCommand(doc.as<JsonVariantConst>());

    TEST_ASSERT_FALSE(result.ok);
}

void test_parse_mqtt_command_propagates_underlying_parse_error() {
    JsonDocument doc;
    doc["type"] = "look";
    doc["x"] = 0.1f;
    // y missing -- parseLookCommand itself should fail

    const MqttParseResult result = parseMqttCommand(doc.as<JsonVariantConst>());

    TEST_ASSERT_FALSE(result.ok);
    TEST_ASSERT_TRUE(result.error.size() > 0);
}

// No main() here -- test/test_native/test_main.cpp is the sole file with main().
