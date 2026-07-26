#include <unity.h>

#include <ArduinoJson.h>

#include "EyeCommandJson.h"

using eyesee::CommandType;
using eyesee::EyeCommand;
using eyesee::Expression;
using eyesee::ParseResult;
using eyesee::parseBlinkCommand;
using eyesee::parseExpressionCommand;
using eyesee::parseLookCommand;
using eyesee::parseWinkCommand;

void test_parse_look_command_reads_required_and_optional_fields() {
    JsonDocument doc;
    doc["x"] = 0.4f;
    doc["y"] = -0.2f;
    doc["speed"] = 500.0f;
    doc["blinkOnArrival"] = true;
    doc["hold"] = true;

    EyeCommand command;
    const ParseResult result = parseLookCommand(doc.as<JsonVariantConst>(), command);

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Look), static_cast<int>(command.type));
    TEST_ASSERT_EQUAL_FLOAT(0.4f, command.gazeTarget.x);
    TEST_ASSERT_EQUAL_FLOAT(-0.2f, command.gazeTarget.y);
    TEST_ASSERT_EQUAL_FLOAT(500.0f, command.gazeTarget.speed);
    TEST_ASSERT_TRUE(command.gazeTarget.blinkOnArrival);
    TEST_ASSERT_TRUE(command.gazeTarget.hold);
}

void test_parse_look_command_defaults_optional_fields() {
    JsonDocument doc;
    doc["x"] = 0.1f;
    doc["y"] = 0.1f;

    EyeCommand command;
    const ParseResult result = parseLookCommand(doc.as<JsonVariantConst>(), command);

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_FLOAT(300.0f, command.gazeTarget.speed);
    TEST_ASSERT_FALSE(command.gazeTarget.blinkOnArrival);
    TEST_ASSERT_FALSE(command.gazeTarget.hold);
}

void test_parse_look_command_clamps_out_of_range_xy() {
    JsonDocument doc;
    doc["x"] = 5.0f;
    doc["y"] = -5.0f;

    EyeCommand command;
    const ParseResult result = parseLookCommand(doc.as<JsonVariantConst>(), command);

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, command.gazeTarget.x);
    TEST_ASSERT_EQUAL_FLOAT(-1.0f, command.gazeTarget.y);
}

void test_parse_look_command_rejects_missing_x() {
    JsonDocument doc;
    doc["y"] = 0.1f;

    EyeCommand command;
    const ParseResult result = parseLookCommand(doc.as<JsonVariantConst>(), command);

    TEST_ASSERT_FALSE(result.ok);
    TEST_ASSERT_TRUE(result.error.size() > 0);
}

void test_parse_blink_command_defaults_duration_when_absent() {
    JsonDocument doc;  // {}

    EyeCommand command;
    const ParseResult result = parseBlinkCommand(doc.as<JsonVariantConst>(), command);

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::Blink), static_cast<int>(command.type));
    TEST_ASSERT_EQUAL_UINT32(150, command.durationMs);
}

void test_parse_blink_command_reads_custom_duration() {
    JsonDocument doc;
    doc["durationMs"] = 300;

    EyeCommand command;
    const ParseResult result = parseBlinkCommand(doc.as<JsonVariantConst>(), command);

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_UINT32(300, command.durationMs);
}

void test_parse_wink_command_left() {
    JsonDocument doc;
    doc["side"] = "left";

    EyeCommand command;
    const ParseResult result = parseWinkCommand(doc.as<JsonVariantConst>(), command);

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::WinkLeft), static_cast<int>(command.type));
}

void test_parse_wink_command_right() {
    JsonDocument doc;
    doc["side"] = "right";

    EyeCommand command;
    const ParseResult result = parseWinkCommand(doc.as<JsonVariantConst>(), command);

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::WinkRight), static_cast<int>(command.type));
}

void test_parse_wink_command_rejects_invalid_side() {
    JsonDocument doc;
    doc["side"] = "up";

    EyeCommand command;
    const ParseResult result = parseWinkCommand(doc.as<JsonVariantConst>(), command);

    TEST_ASSERT_FALSE(result.ok);
}

void test_parse_wink_command_rejects_missing_side() {
    JsonDocument doc;  // {}

    EyeCommand command;
    const ParseResult result = parseWinkCommand(doc.as<JsonVariantConst>(), command);

    TEST_ASSERT_FALSE(result.ok);
}

void test_parse_expression_command_reads_known_name() {
    JsonDocument doc;
    doc["expression"] = "Happy";

    EyeCommand command;
    const ParseResult result = parseExpressionCommand(doc.as<JsonVariantConst>(), command);

    TEST_ASSERT_TRUE(result.ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(CommandType::SetExpression), static_cast<int>(command.type));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(Expression::Happy), static_cast<int>(command.expression));
}

void test_parse_expression_command_rejects_unknown_name() {
    JsonDocument doc;
    doc["expression"] = "Grumpy";

    EyeCommand command;
    const ParseResult result = parseExpressionCommand(doc.as<JsonVariantConst>(), command);

    TEST_ASSERT_FALSE(result.ok);
}

// No main() here — test/test_native/test_main.cpp is the sole file with main().
